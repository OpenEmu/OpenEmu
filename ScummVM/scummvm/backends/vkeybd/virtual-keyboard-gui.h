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
*
*/

#ifndef COMMON_VIRTUAL_KEYBOARD_GUI_H
#define COMMON_VIRTUAL_KEYBOARD_GUI_H

#include "common/scummsys.h"

#ifdef ENABLE_VKEYBD

#include "backends/vkeybd/virtual-keyboard.h"
#include "common/rect.h"
#include "common/system.h"
#include "graphics/font.h"
#include "graphics/surface.h"

namespace Common {

/**
 * Class to handle the drawing of the virtual keyboard to the overlay, and the
 * execution of the keyboard's main loop.
 * This includes the blitting of the appropriate bitmap in the correct location,
 * as well as the drawing of the virtual keyboard display.
 */
class VirtualKeyboardGUI {

public:

	VirtualKeyboardGUI(VirtualKeyboard *kbd);
	~VirtualKeyboardGUI();

	/**
	 * Updates the GUI when the Mode of the keyboard is changes
	 */
	void initMode(VirtualKeyboard::Mode *mode);

	/**
	 * Starts the drawing of the keyboard, and runs the main event loop.
	 */
	void run();

	/**
	 * Interrupts the event loop and resets the overlay to its initial state.
	 */
	void close();

	bool isDisplaying() { return _displaying; }

	/**
	 * Reset the class to an initial state
	 */
	void reset();

	/**
	 * Activates drag mode. Takes the keyboard-relative coordinates of the
	 * cursor as an argument.
	 */
	void startDrag(int16 x, int16 y);

	/**
	 * Deactivates drag mode
	 * */
	void endDrag();

	/**
	 * Checks for a screen change in the backend and re-inits the virtual
	 * keyboard if it has.
	 */
	void checkScreenChanged();

	/**
	 * Sets the GUI's internal screen size variables
	 */
	void initSize(int16 w, int16 h);

private:

	OSystem *_system;

	VirtualKeyboard *_kbd;
	Rect _kbdBound;
	Graphics::Surface *_kbdSurface;
	OverlayColor _kbdTransparentColor;

	Point _dragPoint;
	bool _drag;
	static const int SNAP_WIDTH = 10;

	Graphics::Surface _overlayBackup;
	Rect _dirtyRect;

	bool _displayEnabled;
	Graphics::Surface _dispSurface;
	const Graphics::Font *_dispFont;
	int16 _dispX, _dispY;
	uint _dispI;
	OverlayColor _dispForeColor, _dispBackColor;

	int _lastScreenChanged;
	int16 _screenW, _screenH;

	bool _displaying;
	bool _firstRun;

	void setupDisplayArea(Rect &r, OverlayColor forecolor);
	void move(int16 x, int16 y);
	void moveToDefaultPosition();
	void screenChanged();
	void mainLoop();
	void extendDirtyRect(const Rect &r);
	void resetDirtyRect();
	void redraw();
	void forceRedraw();
	void updateDisplay();
	bool fontIsSuitable(const Graphics::Font *font, const Rect &rect);
	uint calculateEndIndex(const String &str, uint startIndex);

	bool _drawCaret;
	int16 _caretX;
	static const int kCaretBlinkTime = 500;
	void animateCaret();

	static const int kCursorAnimateDelay = 250;
	int _cursorAnimateCounter;
	int _cursorAnimateTimer;
	byte _cursor[2048];
	void setupCursor();
	void removeCursor();
	void animateCursor();

};

} // End of namespace Common

#endif // #ifdef ENABLE_VKEYBD

#endif // #ifndef COMMON_VIRTUAL_KEYBOARD_GUI_H
