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

#include "common/str.h"
#include "common/system.h"
#include "gui/message.h"
#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"
#include "gui/widget.h"

namespace GUI {

enum {
	kOkCmd = 'OK  ',
	kCancelCmd = 'CNCL'
};


// TODO: The default button should be visibly distinct from the alternate button

MessageDialog::MessageDialog(const Common::String &message, const char *defaultButton, const char *altButton)
	: Dialog(30, 20, 260, 124) {

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	int buttonWidth = g_gui.xmlEval()->getVar("Globals.Button.Width", 0);
	int buttonHeight = g_gui.xmlEval()->getVar("Globals.Button.Height", 0);

	// First, determine the size the dialog needs. For this we have to break
	// down the string into lines, and taking the maximum of their widths.
	// Using this, and accounting for the space the button(s) need, we can set
	// the real size of the dialog
	Common::Array<Common::String> lines;
	int lineCount, okButtonPos, cancelButtonPos;
	int maxlineWidth = g_gui.getFont().wordWrapText(message, screenW - 2 * 20, lines);

	// Calculate the desired dialog size (maxing out at 300*180 for now)
	if (altButton)
		_w = MAX(maxlineWidth, (2 * buttonWidth) + 10) + 20;
	else
		_w = MAX(maxlineWidth, buttonWidth) + 20;

	lineCount = lines.size();

	_h = 16;
	if (defaultButton || altButton)
		_h += buttonHeight + 8;

	// Limit the number of lines so that the dialog still fits on the screen.
	if (lineCount > (screenH - 20 - _h) / kLineHeight) {
		lineCount = (screenH - 20 - _h) / kLineHeight;
	}
	_h += lineCount * kLineHeight;

	// Center the dialog
	_x = (screenW - _w) / 2;
	_y = (screenH - _h) / 2;

	// Each line is represented by one static text item.
	for (int i = 0; i < lineCount; i++) {
		new StaticTextWidget(this, 10, 10 + i * kLineHeight, maxlineWidth, kLineHeight,
								lines[i], Graphics::kTextAlignCenter);
	}

	if (defaultButton && altButton) {
		okButtonPos = (_w - (buttonWidth * 2)) / 2;
		cancelButtonPos = ((_w - (buttonWidth * 2)) / 2) + buttonWidth + 10;
	} else {
		okButtonPos = cancelButtonPos = (_w - buttonWidth) / 2;
	}

	if (defaultButton)
		new ButtonWidget(this, okButtonPos, _h - buttonHeight - 8, buttonWidth, buttonHeight, defaultButton, 0, kOkCmd, Common::ASCII_RETURN);	// Confirm dialog

	if (altButton)
		new ButtonWidget(this, cancelButtonPos, _h - buttonHeight - 8, buttonWidth, buttonHeight, altButton, 0, kCancelCmd, Common::ASCII_ESCAPE);	// Cancel dialog
}

void MessageDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	// FIXME: It's a really bad thing that we use two arbitrary constants
	if (cmd == kOkCmd) {
		setResult(kMessageOK);
		close();
	} else if (cmd == kCancelCmd) {
		setResult(kMessageCancel);
		close();
	} else {
		Dialog::handleCommand(sender, cmd, data);
	}
}

TimedMessageDialog::TimedMessageDialog(const Common::String &message, uint32 duration)
	: MessageDialog(message, 0, 0) {
	_timer = g_system->getMillis() + duration;
}

void TimedMessageDialog::handleTickle() {
	MessageDialog::handleTickle();
	if (g_system->getMillis() > _timer)
		close();
}

} // End of namespace GUI
