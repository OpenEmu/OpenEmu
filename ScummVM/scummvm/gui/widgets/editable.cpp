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

#include "common/rect.h"
#include "common/system.h"
#include "gui/widgets/editable.h"
#include "gui/gui-manager.h"
#include "graphics/font.h"

namespace GUI {

EditableWidget::EditableWidget(GuiObject *boss, int x, int y, int w, int h, const char *tooltip, uint32 cmd)
	: Widget(boss, x, y, w, h, tooltip), CommandSender(boss), _cmd(cmd) {
	init();
}

EditableWidget::EditableWidget(GuiObject *boss, const String &name, const char *tooltip, uint32 cmd)
	: Widget(boss, name, tooltip), CommandSender(boss), _cmd(cmd) {
	init();
}

void EditableWidget::init() {
	_caretVisible = false;
	_caretTime = 0;
	_caretPos = 0;

	_caretInverse = false;

	_editScrollOffset = 0;

	_font = ThemeEngine::kFontStyleBold;
	_inversion = ThemeEngine::kTextInversionNone;
}

EditableWidget::~EditableWidget() {
}

void EditableWidget::reflowLayout() {
	Widget::reflowLayout();

	_editScrollOffset = g_gui.getStringWidth(_editString, _font) - getEditRect().width();
	if (_editScrollOffset < 0)
		_editScrollOffset = 0;
}

void EditableWidget::setEditString(const String &str) {
	// TODO: We probably should filter the input string here,
	// e.g. using tryInsertChar.
	_editString = str;
	_caretPos = 0;
}

bool EditableWidget::tryInsertChar(byte c, int pos) {
	if ((c >= 32 && c <= 127) || c >= 160) {
		_editString.insertChar(c, pos);
		return true;
	}
	return false;
}

void EditableWidget::handleTickle() {
	uint32 time = g_system->getMillis();
	if (_caretTime < time) {
		_caretTime = time + kCaretBlinkTime;
		drawCaret(_caretVisible);
	}
}

bool EditableWidget::handleKeyDown(Common::KeyState state) {
	bool handled = true;
	bool dirty = false;
	bool forcecaret = false;

	// First remove caret
	if (_caretVisible)
		drawCaret(true);

	// Remap numeric keypad if NUM lock is *not* active.
	// This code relies on the fact that the various KEYCODE_KP* values are
	// consecutive.
	if (0 == (state.flags & Common::KBD_NUM)
		&& Common::KEYCODE_KP0 <= state.keycode
		&& state.keycode <= Common::KEYCODE_KP_PERIOD) {
		const Common::KeyCode remap[11] = {
			Common::KEYCODE_INSERT, 	// KEYCODE_KP0
			Common::KEYCODE_END,	 	// KEYCODE_KP1
			Common::KEYCODE_DOWN, 		// KEYCODE_KP2
			Common::KEYCODE_PAGEDOWN, 	// KEYCODE_KP3
			Common::KEYCODE_LEFT, 		// KEYCODE_KP4
			Common::KEYCODE_INVALID, 	// KEYCODE_KP5
			Common::KEYCODE_RIGHT,	 	// KEYCODE_KP6
			Common::KEYCODE_HOME,	 	// KEYCODE_KP7
			Common::KEYCODE_UP, 		// KEYCODE_KP8
			Common::KEYCODE_PAGEUP, 	// KEYCODE_KP9
			Common::KEYCODE_DELETE,	 	// KEYCODE_KP_PERIOD
		};
		state.keycode = remap[state.keycode - Common::KEYCODE_KP0];
	}

	switch (state.keycode) {
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		// confirm edit and exit editmode
		endEditMode();
		dirty = true;
		break;

	case Common::KEYCODE_ESCAPE:
		abortEditMode();
		dirty = true;
		break;

	case Common::KEYCODE_BACKSPACE:
		if (_caretPos > 0) {
			_caretPos--;
			_editString.deleteChar(_caretPos);
			dirty = true;

			sendCommand(_cmd, 0);
		}
		forcecaret = true;
		break;

	case Common::KEYCODE_DELETE:
		if (_caretPos < (int)_editString.size()) {
			_editString.deleteChar(_caretPos);
			dirty = true;

			sendCommand(_cmd, 0);
		}
		forcecaret = true;
		break;

	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_END:
		// Move caret to end
		dirty = setCaretPos(_editString.size());
		forcecaret = true;
		break;

	case Common::KEYCODE_LEFT:
		// Move caret one left (if possible)
		if (_caretPos > 0) {
			dirty = setCaretPos(_caretPos - 1);
		}
		forcecaret = true;
		dirty = true;
		break;

	case Common::KEYCODE_RIGHT:
		// Move caret one right (if possible)
		if (_caretPos < (int)_editString.size()) {
			dirty = setCaretPos(_caretPos + 1);
		}
		forcecaret = true;
		dirty = true;
		break;

	case Common::KEYCODE_UP:
	case Common::KEYCODE_HOME:
		// Move caret to start
		dirty = setCaretPos(0);
		forcecaret = true;
		break;

#ifdef MACOSX
	// Let ctrl-a / ctrl-e move the caret to the start / end of the line.
	//
	// These shortcuts go back a long time for command line programs. As
	// for edit fields in GUIs, they are supported natively on Mac OS X,
	// which is why I enabled these shortcuts there.
	// On other systems (Windows, Gnome), Ctrl-A by default means
	// "select all", which is why I didn't enable the shortcuts there
	// for now, to avoid potential confusion.
	//
	// But since we don't support text selection, and since at least Gnome
	// can be configured to also support ctrl-a and ctrl-e, we may want
	// to extend this code to other targets, maybe even all. I'll leave
	// this to other porters to decide, though.
	case Common::KEYCODE_a:
	case Common::KEYCODE_e:
		if (state.flags & Common::KBD_CTRL) {
			if (state.keycode == Common::KEYCODE_a) {
				// Move caret to start
				dirty = setCaretPos(0);
				forcecaret = true;
			} else if (state.keycode == Common::KEYCODE_e) {
				// Move caret to end
				dirty = setCaretPos(_editString.size());
				forcecaret = true;
			}
			break;
		}
#endif

	default:
		defaultKeyDownHandler(state, dirty, forcecaret, handled);
	}

	if (dirty)
		draw();

	if (forcecaret)
		makeCaretVisible();

	return handled;
}

void EditableWidget::defaultKeyDownHandler(Common::KeyState &state, bool &dirty, bool &forcecaret, bool &handled) {
	if (state.ascii < 256 && tryInsertChar((byte)state.ascii, _caretPos)) {
		_caretPos++;
		dirty = true;
		forcecaret = true;

		sendCommand(_cmd, 0);
	} else {
		handled = false;
	}
}

int EditableWidget::getCaretOffset() const {
	int caretpos = 0;

	uint last = 0;
	for (int i = 0; i < _caretPos; ++i) {
		const uint cur = _editString[i];
		caretpos += g_gui.getCharWidth(cur, _font) + g_gui.getKerningOffset(last, cur, _font);
		last = cur;
	}

	caretpos -= _editScrollOffset;

	return caretpos;
}

void EditableWidget::drawCaret(bool erase) {
	// Only draw if item is visible
	if (!isVisible() || !_boss->isVisible())
		return;

	Common::Rect editRect = getEditRect();

	int x = editRect.left;
	int y = editRect.top;

	x += getCaretOffset();

	if (y < 0 || y + editRect.height() - 2 >= _h)
		return;

	x += getAbsX();
	y += getAbsY();

	g_gui.theme()->drawCaret(Common::Rect(x, y, x + 1, y + editRect.height() - 2), erase);

	if (erase) {
		if ((uint)_caretPos < _editString.size()) {
			GUI::EditableWidget::String chr(_editString[_caretPos]);
			int chrWidth = g_gui.getCharWidth(_editString[_caretPos], _font);
			const uint last = (_caretPos > 0) ? _editString[_caretPos - 1] : 0;
			x += g_gui.getKerningOffset(last, _editString[_caretPos], _font);
			g_gui.theme()->drawText(Common::Rect(x, y, x + chrWidth, y + editRect.height() - 2), chr, _state, Graphics::kTextAlignLeft, _inversion, 0, false, _font);
		}
	}

	_caretVisible = !erase;
}

bool EditableWidget::setCaretPos(int newPos) {
	assert(newPos >= 0 && newPos <= (int)_editString.size());
	_caretPos = newPos;
	return adjustOffset();
}

bool EditableWidget::adjustOffset() {
	// check if the caret is still within the textbox; if it isn't,
	// adjust _editScrollOffset

	int caretpos = getCaretOffset();
	const int editWidth = getEditRect().width();

	if (caretpos < 0) {
		// scroll left
		_editScrollOffset += caretpos;
		return true;
	} else if (caretpos >= editWidth) {
		// scroll right
		_editScrollOffset -= (editWidth - caretpos);
		return true;
	} else if (_editScrollOffset > 0) {
		const int strWidth = g_gui.getStringWidth(_editString, _font);
		if (strWidth - _editScrollOffset < editWidth) {
			// scroll right
			_editScrollOffset = (strWidth - editWidth);
			if (_editScrollOffset < 0)
				_editScrollOffset = 0;
		}
	}

	return false;
}

void EditableWidget::makeCaretVisible() {
	_caretTime = g_system->getMillis() + kCaretBlinkTime;
	_caretVisible = true;
	drawCaret(false);
}

} // End of namespace GUI
