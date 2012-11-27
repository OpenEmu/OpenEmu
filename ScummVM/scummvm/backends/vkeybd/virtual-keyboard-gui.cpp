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

#include "common/scummsys.h"

#ifdef ENABLE_VKEYBD

#include "backends/vkeybd/virtual-keyboard-gui.h"

#include "graphics/cursorman.h"
#include "graphics/fontman.h"
#include "gui/gui-manager.h"

namespace Common {

static void blit(Graphics::Surface *surf_dst, Graphics::Surface *surf_src, int16 x, int16 y, OverlayColor transparent) {
	if (surf_dst->format.bytesPerPixel != sizeof(OverlayColor) || surf_src->format.bytesPerPixel != sizeof(OverlayColor))
		return;

	const OverlayColor *src = (const OverlayColor *)surf_src->pixels;
	int blitW = surf_src->w;
	int blitH = surf_src->h;

	// clip co-ordinates
	if (x < 0) {
		blitW += x;
		src -= x;
		x = 0;
	}
	if (y < 0) {
		blitH += y;
		src -= y * surf_src->w;
		y = 0;
	}
	if (blitW > surf_dst->w - x)
		blitW = surf_dst->w - x;
	if (blitH > surf_dst->h - y)
		blitH = surf_dst->h - y;
	if (blitW <= 0 || blitH <= 0)
		return;

	OverlayColor *dst = (OverlayColor *)surf_dst->getBasePtr(x, y);
	int dstAdd = surf_dst->w - blitW;
	int srcAdd = surf_src->w - blitW;

	for (int i = 0; i < blitH; ++i) {
		for (int j = 0; j < blitW; ++j, ++dst, ++src) {
			OverlayColor col = *src;
			if (col != transparent)
				*dst = col;
		}
		dst += dstAdd;
		src += srcAdd;
	}
}

VirtualKeyboardGUI::VirtualKeyboardGUI(VirtualKeyboard *kbd)
	: _kbd(kbd), _displaying(false), _drag(false),
	  _drawCaret(false), _displayEnabled(false), _firstRun(true),
	  _cursorAnimateTimer(0), _cursorAnimateCounter(0) {

	assert(_kbd);
	assert(g_system);
	_system = g_system;

	_lastScreenChanged = _system->getScreenChangeID();
	_screenW = _system->getOverlayWidth();
	_screenH = _system->getOverlayHeight();


	memset(_cursor, 0xFF, sizeof(_cursor));
}

VirtualKeyboardGUI::~VirtualKeyboardGUI() {
	_overlayBackup.free();
	_dispSurface.free();
}

void VirtualKeyboardGUI::initMode(VirtualKeyboard::Mode *mode) {
	assert(mode->image);

	_kbdSurface = mode->image;
	_kbdTransparentColor = mode->transparentColor;
	_kbdBound.setWidth(_kbdSurface->w);
	_kbdBound.setHeight(_kbdSurface->h);

	setupDisplayArea(mode->displayArea, mode->displayFontColor);

	if (_displaying) {
		extendDirtyRect(_kbdBound);
		redraw();
	}
}

void VirtualKeyboardGUI::setupDisplayArea(Rect &r, OverlayColor forecolor) {

	_dispFont = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	if (!fontIsSuitable(_dispFont, r)) {
		_dispFont = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
		if (!fontIsSuitable(_dispFont, r)) {
			/* FIXME: We 'ab'use the kConsoleFont to get a font that fits in a small display_area on 320*240 keyboard images */
			_dispFont = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);
			if (!fontIsSuitable(_dispFont, r)) {
				_displayEnabled = false;
				return;
			}
		}
	}
	_dispX = _kbdBound.left + r.left;
	_dispY = _kbdBound.top + r.top + (r.height() - _dispFont->getFontHeight()) / 2;
	_dispI = 0;
	_dispForeColor = forecolor;
	_dispBackColor = _dispForeColor + 0xFF;
	_dispSurface.create(r.width(), _dispFont->getFontHeight(), _system->getOverlayFormat());
	_dispSurface.fillRect(Rect(_dispSurface.w, _dispSurface.h), _dispBackColor);
	_displayEnabled = true;
}

bool VirtualKeyboardGUI::fontIsSuitable(const Graphics::Font *font, const Rect &rect) {
	return (font->getMaxCharWidth() < rect.width() &&
	        font->getFontHeight() < rect.height());
}

void VirtualKeyboardGUI::checkScreenChanged() {
	if (_lastScreenChanged != _system->getScreenChangeID())
		screenChanged();
}

void VirtualKeyboardGUI::initSize(int16 w, int16 h) {
	_screenW = w;
	_screenH = h;
}

void VirtualKeyboardGUI::run() {
	if (_firstRun) {
		_firstRun = false;
		moveToDefaultPosition();
	}

	if (!g_gui.isActive()) {
		_system->showOverlay();
		_system->clearOverlay();
	}
	_overlayBackup.create(_screenW, _screenH, _system->getOverlayFormat());
	_system->grabOverlay(_overlayBackup.pixels, _overlayBackup.pitch);

	setupCursor();

	forceRedraw();
	_displaying = true;
	mainLoop();

	removeCursor();

	_system->copyRectToOverlay(_overlayBackup.pixels, _overlayBackup.pitch, 0, 0, _overlayBackup.w, _overlayBackup.h);
	if (!g_gui.isActive()) _system->hideOverlay();

	_overlayBackup.free();
	_dispSurface.free();
}

void VirtualKeyboardGUI::close() {
	_displaying = false;
}

void VirtualKeyboardGUI::reset() {
	_kbdBound.left = _kbdBound.top = 0;
	_kbdBound.right = _kbdBound.bottom = 0;
	_displaying = _drag = false;
	_firstRun = true;
	_lastScreenChanged = _system->getScreenChangeID();
	_kbdSurface = 0;
}

void VirtualKeyboardGUI::moveToDefaultPosition() {
	int16 kbdW = _kbdBound.width(), kbdH = _kbdBound.height();
	int16 x = 0, y = 0;
	if (_screenW != kbdW) {
		switch (_kbd->_hAlignment) {
		case VirtualKeyboard::kAlignLeft:
			x = 0;
			break;
		case VirtualKeyboard::kAlignCenter:
			x = (_screenW - kbdW) / 2;
			break;
		case VirtualKeyboard::kAlignRight:
			x = _screenW - kbdW;
			break;
		}
	}
	if (_screenH != kbdH) {
		switch (_kbd->_vAlignment) {
		case VirtualKeyboard::kAlignTop:
			y = 0;
			break;
		case VirtualKeyboard::kAlignMiddle:
			y = (_screenH - kbdH) / 2;
			break;
		case VirtualKeyboard::kAlignBottom:
			y = _screenH - kbdH;
			break;
		}
	}
	move(x, y);
}

void VirtualKeyboardGUI::move(int16 x, int16 y) {
	// add old position to dirty area
	if (_displaying) extendDirtyRect(_kbdBound);

	// snap to edge of screen
	if (ABS(x) < SNAP_WIDTH)
		x = 0;
	int16 x2 = _screenW - _kbdBound.width();
	if (ABS(x - x2) < SNAP_WIDTH)
		x = x2;
	if (ABS(y) < SNAP_WIDTH)
		y = 0;
	int16 y2 = _screenH - _kbdBound.height();
	if (ABS(y - y2) < SNAP_WIDTH)
		y = y2;

	_dispX += x - _kbdBound.left;
	_dispY += y - _kbdBound.top;
	_kbdBound.moveTo(x, y);

	if (_displaying) {
		// add new position to dirty area
		extendDirtyRect(_kbdBound);
		redraw();
	}
}

void VirtualKeyboardGUI::screenChanged() {
	g_gui.checkScreenChange();

	_lastScreenChanged = _system->getScreenChangeID();
	int16 newScreenW = _system->getOverlayWidth();
	int16 newScreenH = _system->getOverlayHeight();

	if (_screenW != newScreenW || _screenH != newScreenH) {
		_screenW = newScreenW;
		_screenH = newScreenH;

		_overlayBackup.create(_screenW, _screenH, _system->getOverlayFormat());
		_system->grabOverlay(_overlayBackup.pixels, _overlayBackup.pitch);

		if (!_kbd->checkModeResolutions()) {
			_displaying = false;
			return;
		}
		moveToDefaultPosition();
	}
}


void VirtualKeyboardGUI::mainLoop() {
	Common::EventManager *eventMan = _system->getEventManager();

	while (_displaying) {
		if (_kbd->_keyQueue.hasStringChanged())
			updateDisplay();
		animateCaret();
		animateCursor();
		redraw();
		_system->updateScreen();
		Common::Event event;
		while (eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_LBUTTONDOWN:
				if (_kbdBound.contains(event.mouse)) {
					_kbd->handleMouseDown(event.mouse.x - _kbdBound.left,
					                      event.mouse.y - _kbdBound.top);
				}
				break;
			case Common::EVENT_LBUTTONUP:
				if (_kbdBound.contains(event.mouse)) {
					_kbd->handleMouseUp(event.mouse.x - _kbdBound.left,
					                    event.mouse.y - _kbdBound.top);
				}
				break;
			case Common::EVENT_MOUSEMOVE:
				if (_drag)
					move(event.mouse.x - _dragPoint.x,
					     event.mouse.y - _dragPoint.y);
				break;
			case Common::EVENT_SCREEN_CHANGED:
				screenChanged();
				break;
			case Common::EVENT_QUIT:
				_system->quit();
				return;
			default:
				break;
			}
		}
		// Delay for a moment
		_system->delayMillis(10);
	}
}

void VirtualKeyboardGUI::startDrag(int16 x, int16 y) {
	_drag = true;
	_dragPoint.x = x;
	_dragPoint.y = y;
}

void VirtualKeyboardGUI::endDrag() {
	_drag = false;
}

void VirtualKeyboardGUI::extendDirtyRect(const Rect &r) {
	if (_dirtyRect.isValidRect()) {
		_dirtyRect.extend(r);
	} else {
		_dirtyRect = r;
	}
	_dirtyRect.clip(Rect(_overlayBackup.w, _overlayBackup.h));
}

void VirtualKeyboardGUI::resetDirtyRect() {
	_dirtyRect.setWidth(-1);
}

void VirtualKeyboardGUI::forceRedraw() {
	updateDisplay();
	extendDirtyRect(Rect(_overlayBackup.w, _overlayBackup.h));
	redraw();
}

void VirtualKeyboardGUI::redraw() {
	assert(_kbdSurface);
	int16 w = _dirtyRect.width();
	int16 h = _dirtyRect.height();
	if (w <= 0 || h <= 0) return;

	Graphics::Surface surf;
	surf.create(w, h, _system->getOverlayFormat());

	OverlayColor *dst = (OverlayColor *)surf.pixels;
	const OverlayColor *src = (OverlayColor *) _overlayBackup.getBasePtr(_dirtyRect.left, _dirtyRect.top);

	while (h--) {
		memcpy(dst, src, surf.w * sizeof(OverlayColor));
		dst += surf.w;
		src += _overlayBackup.w;
	}

	blit(&surf, _kbdSurface, _kbdBound.left - _dirtyRect.left,
	     _kbdBound.top - _dirtyRect.top, _kbdTransparentColor);
	if (_displayEnabled) {
		blit(&surf, &_dispSurface, _dispX - _dirtyRect.left,
		     _dispY - _dirtyRect.top, _dispBackColor);
	}
	_system->copyRectToOverlay(surf.pixels, surf.pitch,
	                           _dirtyRect.left, _dirtyRect.top, surf.w, surf.h);

	surf.free();

	resetDirtyRect();
}

uint VirtualKeyboardGUI::calculateEndIndex(const String &str, uint startIndex) {
	int16 w = 0;
	while (w <= _dispSurface.w && startIndex < str.size()) {
		w += _dispFont->getCharWidth(str[startIndex++]);
	}
	if (w > _dispSurface.w) startIndex--;
	return startIndex;
}

void VirtualKeyboardGUI::animateCaret() {
	if (!_displayEnabled) return;

	if (_system->getMillis() % kCaretBlinkTime < kCaretBlinkTime / 2) {
		if (!_drawCaret) {
			_drawCaret = true;
			_dispSurface.drawLine(_caretX, 0, _caretX, _dispSurface.h, _dispForeColor);
			extendDirtyRect(Rect(_dispX + _caretX, _dispY, _dispX + _caretX + 1, _dispY + _dispSurface.h));
		}
	} else {
		if (_drawCaret) {
			_drawCaret = false;
			_dispSurface.drawLine(_caretX, 0, _caretX, _dispSurface.h, _dispBackColor);
			extendDirtyRect(Rect(_dispX + _caretX, _dispY, _dispX + _caretX + 1, _dispY + _dispSurface.h));
		}
	}
}

void VirtualKeyboardGUI::updateDisplay() {
	if (!_displayEnabled) return;

	// calculate the text to display
	uint cursorPos = _kbd->_keyQueue.getInsertIndex();
	String wholeText = _kbd->_keyQueue.getString();
	uint dispTextEnd;
	if (_dispI > cursorPos)
		_dispI = cursorPos;

	dispTextEnd = calculateEndIndex(wholeText, _dispI);
	while (cursorPos > dispTextEnd)
		dispTextEnd = calculateEndIndex(wholeText, ++_dispI);

	String dispText = String(wholeText.c_str() + _dispI, wholeText.c_str() + dispTextEnd);

	// draw to display surface
	_dispSurface.fillRect(Rect(_dispSurface.w, _dispSurface.h), _dispBackColor);
	_dispFont->drawString(&_dispSurface, dispText, 0, 0, _dispSurface.w, _dispForeColor);

	String beforeCaret(wholeText.c_str() + _dispI, wholeText.c_str() + cursorPos);
	_caretX = _dispFont->getStringWidth(beforeCaret);
	if (_drawCaret) _dispSurface.drawLine(_caretX, 0, _caretX, _dispSurface.h, _dispForeColor);

	extendDirtyRect(Rect(_dispX, _dispY, _dispX + _dispSurface.w, _dispY + _dispSurface.h));
}

void VirtualKeyboardGUI::setupCursor() {
	const byte palette[] = {
		255, 255, 255,
		255, 255, 255,
		171, 171, 171,
		87,  87,  87
	};

	CursorMan.pushCursorPalette(palette, 0, 4);
	CursorMan.pushCursor(NULL, 0, 0, 0, 0, 0);
	CursorMan.showMouse(true);
}

void VirtualKeyboardGUI::animateCursor() {
	int time = _system->getMillis();
	if (time > _cursorAnimateTimer + kCursorAnimateDelay) {
		for (int i = 0; i < 15; i++) {
			if ((i < 6) || (i > 8)) {
				_cursor[16 * 7 + i] = _cursorAnimateCounter;
				_cursor[16 * i + 7] = _cursorAnimateCounter;
			}
		}

		CursorMan.replaceCursor(_cursor, 16, 16, 7, 7, 255);

		_cursorAnimateTimer = time;
		_cursorAnimateCounter = (_cursorAnimateCounter + 1) % 4;
	}
}

void VirtualKeyboardGUI::removeCursor() {
	CursorMan.popCursor();
	CursorMan.popCursorPalette();
}

} // End of namespace Common

#endif // #ifdef ENABLE_VKEYBD
