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

#include "gui/widgets/edittext.h"
#include "gui/gui-manager.h"

#include "gui/ThemeEval.h"

namespace GUI {

EditTextWidget::EditTextWidget(GuiObject *boss, int x, int y, int w, int h, const String &text, const char *tooltip, uint32 cmd, uint32 finishCmd)
	: EditableWidget(boss, x, y - 1, w, h + 2, tooltip, cmd) {
	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE);
	_type = kEditTextWidget;
	_finishCmd = finishCmd;

	setEditString(text);
	setFontStyle(ThemeEngine::kFontStyleNormal);
}

EditTextWidget::EditTextWidget(GuiObject *boss, const String &name, const String &text, const char *tooltip, uint32 cmd, uint32 finishCmd)
	: EditableWidget(boss, name, tooltip, cmd) {
	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE);
	_type = kEditTextWidget;
	_finishCmd = finishCmd;

	setEditString(text);
	setFontStyle(ThemeEngine::kFontStyleNormal);
}

void EditTextWidget::setEditString(const String &str) {
	EditableWidget::setEditString(str);
	_backupString = str;
}

void EditTextWidget::reflowLayout() {
	_leftPadding = g_gui.xmlEval()->getVar("Globals.EditTextWidget.Padding.Left", 0);
	_rightPadding = g_gui.xmlEval()->getVar("Globals.EditTextWidget.Padding.Right", 0);

	EditableWidget::reflowLayout();
}


void EditTextWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	// First remove caret
	if (_caretVisible)
		drawCaret(true);

	x += _editScrollOffset;

	int width = 0;
	uint i;

	uint last = 0;
	for (i = 0; i < _editString.size(); ++i) {
		const uint cur = _editString[i];
		width += g_gui.getCharWidth(cur, _font) + g_gui.getKerningOffset(last, cur, _font);
		if (width >= x)
			break;
		last = cur;
	}
	if (setCaretPos(i))
		draw();
}


void EditTextWidget::drawWidget() {
	g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x+_w, _y+_h), 0, ThemeEngine::kWidgetBackgroundEditText);

	// Draw the text
	adjustOffset();
	g_gui.theme()->drawText(Common::Rect(_x+2+ _leftPadding,_y+2, _x+_leftPadding+getEditRect().width()+2, _y+_h-2), _editString, _state, Graphics::kTextAlignLeft, ThemeEngine::kTextInversionNone, -_editScrollOffset, false, _font);
}

Common::Rect EditTextWidget::getEditRect() const {
	Common::Rect r(2 + _leftPadding, 2, _w - 2 - _leftPadding - _rightPadding, _h-1);

	return r;
}

void EditTextWidget::receivedFocusWidget() {
}

void EditTextWidget::lostFocusWidget() {
	// If we loose focus, 'commit' the user changes
	_backupString = _editString;
	drawCaret(true);
}

void EditTextWidget::startEditMode() {
}

void EditTextWidget::endEditMode() {
	releaseFocus();

	sendCommand(_finishCmd, 0);
}

void EditTextWidget::abortEditMode() {
	setEditString(_backupString);
	sendCommand(_cmd, 0);
	releaseFocus();
}

} // End of namespace GUI
