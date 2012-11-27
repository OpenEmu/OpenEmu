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

#include "common/util.h"

#include "sci/console.h"
#include "sci/sci.h"
#include "sci/engine/features.h"
#include "sci/engine/gc.h"
#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/paint16.h"
#include "sci/graphics/animate.h"
#include "sci/graphics/text16.h"
#include "sci/graphics/ports.h"

namespace Sci {

GfxPorts::GfxPorts(SegManager *segMan, GfxScreen *screen)
	: _segMan(segMan), _screen(screen) {
}

GfxPorts::~GfxPorts() {
	// reset frees all windows but _picWind
	reset();
	freeWindow(_picWind);
	delete _wmgrPort;
	delete _menuPort;
}

void GfxPorts::init(bool usesOldGfxFunctions, GfxPaint16 *paint16, GfxText16 *text16) {
	int16 offTop = 10;

	_usesOldGfxFunctions = usesOldGfxFunctions;
	_paint16 = paint16;
	_text16 = text16;

	_freeCounter = 0;

	// _menuPort has actually hardcoded id 0xFFFF. Its not meant to be known to windowmanager according to sierra sci
	_menuPort = new Port(0xFFFF);
	openPort(_menuPort);
	setPort(_menuPort);
	_text16->SetFont(0);
	_menuPort->rect = Common::Rect(0, 0, _screen->getWidth(), _screen->getHeight());
	_menuBarRect = Common::Rect(0, 0, _screen->getWidth(), 9);
	_menuRect = Common::Rect(0, 0, _screen->getWidth(), 10);
	_menuLine = Common::Rect(0, 9, _screen->getWidth(), 10);

	_wmgrPort = new Port(1);
	_windowsById.resize(2);
	_windowsById[0] = _wmgrPort; // wmgrPort is supposed to be accessible via id 0
	_windowsById[1] = _wmgrPort; //  but wmgrPort may not actually have id 0, so we assign id 1 (as well)
	// Background: sierra sci replies with the offset of curPort on kGetPort calls. If we reply with 0 there most games
	//				will work, but some scripts seem to check for 0 and initialize the variable again in that case
	//				resulting in problems.

	if (getSciVersion() >= SCI_VERSION_1_LATE)
		_styleUser = SCI_WINDOWMGR_STYLE_USER;
	else
		_styleUser = SCI_WINDOWMGR_STYLE_USER | SCI_WINDOWMGR_STYLE_TRANSPARENT;

	// Jones, Slater, Hoyle 3&4 and Crazy Nicks Laura Bow/Kings Quest were
	// called with parameter -Nw 0 0 200 320.
	// Mother Goose (SCI1) uses -Nw 0 0 159 262. The game will later use
	// SetPort so we don't need to set the other fields.
	// This actually meant not skipping the first 10 pixellines in windowMgrPort
	switch (g_sci->getGameId()) {
	case GID_JONES:
	case GID_SLATER:
	case GID_HOYLE3:
	case GID_HOYLE4:
	case GID_CNICK_LAURABOW:
	case GID_CNICK_KQ:
		offTop = 0;
		break;
	case GID_MOTHERGOOSE256:
		// only the SCI1 and SCI1.1 (VGA) versions need this
		offTop = 0;
		break;
	case GID_FAIRYTALES:
		// Mixed-Up Fairy Tales (& its demo) uses -w 26 0 200 320. If we don't
		// also do this we will get not-fully-removed windows everywhere.
		offTop = 26;
		break;
	default:
		// For Mac games running with a height of 190, we do not have a menu bar
		// so the top offset should be 0.
		if (_screen->getHeight() == 190)
			offTop = 0;
		break;
	}

	openPort(_wmgrPort);
	setPort(_wmgrPort);
	// SCI0 games till kq4 (.502 - not including) did not adjust against _wmgrPort in kNewWindow
	//  We leave _wmgrPort top at 0, so the adjustment wont get done
	if (!g_sci->_features->usesOldGfxFunctions()) {
		setOrigin(0, offTop);
		_wmgrPort->rect.bottom = _screen->getHeight() - offTop;
	} else {
		_wmgrPort->rect.bottom = _screen->getHeight();
	}
	_wmgrPort->rect.right = _screen->getWidth();
	_wmgrPort->rect.moveTo(0, 0);
	_wmgrPort->curTop = 0;
	_wmgrPort->curLeft = 0;
	_windowList.push_front(_wmgrPort);

	_picWind = addWindow(Common::Rect(0, offTop, _screen->getWidth(), _screen->getHeight()), 0, 0, SCI_WINDOWMGR_STYLE_TRANSPARENT | SCI_WINDOWMGR_STYLE_NOFRAME, 0, true);
	// For SCI0 games till kq4 (.502 - not including) we set _picWind top to offTop instead
	//  Because of the menu/status bar
	if (g_sci->_features->usesOldGfxFunctions())
		_picWind->top = offTop;

	kernelInitPriorityBands();
}

// Removes any windows from windowList
//  is used when restoring/restarting the game
//  Sierra SCI actually saved the whole windowList, it seems we don't need to do this at all
//  but in some games there are still windows active when restoring. Leaving those windows open
//  would create all sorts of issues, that's why we remove them
void GfxPorts::reset() {
	setPort(_picWind);

	// free everything after _picWind
	for (uint id = PORTS_FIRSTSCRIPTWINDOWID; id < _windowsById.size(); id++) {
		Window *window = (Window *)_windowsById[id];
		if (window)
			freeWindow(window);
	}
	_freeCounter = 0;
	_windowList.clear();
	_windowList.push_front(_wmgrPort);
	_windowList.push_back(_picWind);
}

void GfxPorts::kernelSetActive(uint16 portId) {
	if (_freeCounter) {
		// Windows waiting to get freed
		for (uint id = PORTS_FIRSTSCRIPTWINDOWID; id < _windowsById.size(); id++) {
			Window *window = (Window *)_windowsById[id];
			if (window) {
				if (window->counterTillFree) {
					window->counterTillFree--;
					if (!window->counterTillFree) {
						freeWindow(window);
						_freeCounter--;
					}
				}
			}
		}
	}

	switch (portId) {
	case 0:
		setPort(_wmgrPort);
		break;
	case 0xFFFF:
		setPort(_menuPort);
		break;
	default: {
		Port *newPort = getPortById(portId);
		if (newPort)
			setPort(newPort);
		else
			error("GfxPorts::kernelSetActive was requested to set invalid port id %d", portId);
	}
	};
}

Common::Rect GfxPorts::kernelGetPicWindow(int16 &picTop, int16 &picLeft) {
	picTop = _picWind->top;
	picLeft = _picWind->left;
	return _picWind->rect;
}

void GfxPorts::kernelSetPicWindow(Common::Rect rect, int16 picTop, int16 picLeft, bool initPriorityBandsFlag) {
	_picWind->rect = rect;
	_picWind->top = picTop;
	_picWind->left = picLeft;
	if (initPriorityBandsFlag)
		kernelInitPriorityBands();
}

reg_t GfxPorts::kernelGetActive() {
	return make_reg(0, getPort()->id);
}

reg_t GfxPorts::kernelNewWindow(Common::Rect dims, Common::Rect restoreRect, uint16 style, int16 priority, int16 colorPen, int16 colorBack, const char *title) {
	Window *wnd = NULL;

	if (restoreRect.bottom != 0 && restoreRect.right != 0)
		wnd = addWindow(dims, &restoreRect, title, style, priority, false);
	else
		wnd = addWindow(dims, NULL, title, style, priority, false);
	wnd->penClr = colorPen;
	wnd->backClr = colorBack;
	drawWindow(wnd);

	return make_reg(0, wnd->id);
}

void GfxPorts::kernelDisposeWindow(uint16 windowId, bool reanimate) {
	Window *wnd = (Window *)getPortById(windowId);
	if (wnd) {
		if (!wnd->counterTillFree) {
			removeWindow(wnd, reanimate);
		} else {
			error("kDisposeWindow: used already disposed window id %d", windowId);
		}
	} else {
		error("kDisposeWindow: used unknown window id %d", windowId);
	}
}

int16 GfxPorts::isFrontWindow(Window *pWnd) {
	return _windowList.back() == pWnd;
}

void GfxPorts::beginUpdate(Window *wnd) {
	Port *oldPort = setPort(_wmgrPort);
	PortList::iterator it = _windowList.reverse_begin();
	const PortList::iterator end = Common::find(_windowList.begin(), _windowList.end(), wnd);
	while (it != end) {
		// We also store Port objects in the window list, but they
		// shouldn't be encountered during this iteration.
		assert((*it)->isWindow());

		updateWindow((Window *)*it);
		--it;
	}
	setPort(oldPort);
}

void GfxPorts::endUpdate(Window *wnd) {
	Port *oldPort = setPort(_wmgrPort);
	const PortList::iterator end = _windowList.end();
	PortList::iterator it = Common::find(_windowList.begin(), end, wnd);

	// wnd has to be in _windowList
	assert(it != end);

	while (++it != end) {
		// We also store Port objects in the window list, but they
		// shouldn't be encountered during this iteration.
		assert((*it)->isWindow());

		updateWindow((Window *)*it);
	}

	if (getSciVersion() < SCI_VERSION_1_EGA_ONLY)
		g_sci->_gfxPaint16->kernelGraphRedrawBox(_curPort->rect);

	setPort(oldPort);
}

Window *GfxPorts::addWindow(const Common::Rect &dims, const Common::Rect *restoreRect, const char *title, uint16 style, int16 priority, bool draw) {
	// Find an unused window/port id
	uint id = PORTS_FIRSTWINDOWID;
	while (id < _windowsById.size() && _windowsById[id]) {
		if (_windowsById[id]->counterTillFree) {
			// port that is already disposed, but not freed yet
			freeWindow((Window *)_windowsById[id]);
			_freeCounter--;
			break; // reuse the handle
			// we do this especially for sq4cd. it creates and disposes the
			//  inventory window all the time, but reuses old handles as well
			//  this worked somewhat under the original interpreter, because
			//  it put the new window where the old was.
		}
		++id;
	}
	if (id == _windowsById.size())
		_windowsById.push_back(0);
	assert(0 < id && id < 0xFFFF);

	Window *pwnd = new Window(id);
	Common::Rect r;

	if (!pwnd) {
		error("Can't open window");
		return 0;
	}

	_windowsById[id] = pwnd;

	// KQ1sci, KQ4, iceman, QfG2 always add windows to the back of the list.
	// KQ5CD checks style.
	// Hoyle3-demo also always adds to the back (#3036763).
	bool forceToBack = (getSciVersion() <= SCI_VERSION_1_EGA_ONLY) ||
	                   (g_sci->getGameId() == GID_HOYLE3 && g_sci->isDemo());

	if (!forceToBack && (style & SCI_WINDOWMGR_STYLE_TOPMOST))
		_windowList.push_front(pwnd);
	else
		_windowList.push_back(pwnd);
	openPort(pwnd);

	r = dims;
	// This looks fishy, but it's exactly what Sierra did. They removed last
	// bit of the left dimension in their interpreter. It seems Sierra did it
	// for EGA byte alignment (EGA uses 1 byte for 2 pixels) and left it in
	// their interpreter even in the newer VGA games.
	r.left = r.left & 0xFFFE;

	if (r.width() > _screen->getWidth()) {
		// We get invalid dimensions at least at the end of sq3 (script bug!).
		// Same happens very often in lsl5, sierra sci didnt fix it but it looked awful.
		// Also happens frequently in the demo of GK1.
		warning("Fixing too large window, left: %d, right: %d", dims.left, dims.right);
		r.left = 0;
		r.right = _screen->getWidth() - 1;
		if ((style != _styleUser) && !(style & SCI_WINDOWMGR_STYLE_NOFRAME))
			r.right--;
	}
	pwnd->rect = r;
	if (restoreRect)
		pwnd->restoreRect = *restoreRect;

	pwnd->wndStyle = style;
	pwnd->hSaved1 = pwnd->hSaved2 = NULL_REG;
	pwnd->bDrawn = false;
	if ((style & SCI_WINDOWMGR_STYLE_TRANSPARENT) == 0)
		pwnd->saveScreenMask = (priority == -1 ? GFX_SCREEN_MASK_VISUAL : GFX_SCREEN_MASK_VISUAL | GFX_SCREEN_MASK_PRIORITY);

	if (title && (style & SCI_WINDOWMGR_STYLE_TITLE)) {
		pwnd->title = title;
	}

	r = pwnd->rect;
	if ((style != _styleUser) && !(style & SCI_WINDOWMGR_STYLE_NOFRAME)) {
		r.grow(1);
		if (style & SCI_WINDOWMGR_STYLE_TITLE) {
			r.top -= 10;
			r.bottom++;
		}
	}

	pwnd->dims = r;

	// Clip window, if needed
	Common::Rect wmprect = _wmgrPort->rect;
	// Handle a special case for Dr. Brain 1 Mac. When hovering the mouse cursor
	// over the status line on top, the game scripts try to draw the game's icon
	// bar above the current port, by specifying a negative window top, so that
	// the end result will be drawn 10 pixels above the current port. This is a
	// hack by Sierra, and is only limited to user style windows. Normally, we
	// should not clip, same as what Sierra does. However, this will result in
	// having invalid rectangles with negative coordinates. For this reason, we
	// adjust the containing rectangle instead.
	if (pwnd->dims.top < 0 && g_sci->getPlatform() == Common::kPlatformMacintosh &&
		(style & SCI_WINDOWMGR_STYLE_USER) && _wmgrPort->top + pwnd->dims.top >= 0) {
		// Offset the final rect top by the requested pixels
		wmprect.top += pwnd->dims.top;
	}

	int16 oldtop = pwnd->dims.top;
	int16 oldleft = pwnd->dims.left;

	// WORKAROUND: We also adjust the restore rect when adjusting the window
	// rect.
	// SSCI does not do this. It wasn't necessary in the original interpreter,
	// but it is needed for Freddy Pharkas CD. This version does not normally
	// have text, but we allow this by modifying the text/speech setting
	// according to what is set in the ScummVM GUI (refer to syncIngameAudioOptions()
	// in sci.cpp). Since the text used in Freddy Pharkas CD is quite large in
	// some cases, it ends up being offset in order to fit inside the screen,
	// but the associated restore rect isn't adjusted accordingly, leading to
	// artifacts being left on screen when some text boxes are removed. The
	// fact that the restore rect wasn't ever adjusted doesn't make sense, and
	// adjusting it shouldn't have any negative side-effects (it *should* be
	// adjusted, normally, but SCI doesn't do it). The big text boxes are still
	// odd-looking, because the text rect is drawn outside the text window rect,
	// but at least there aren't any leftover textbox artifacts left when the
	// boxes are removed. Adjusting the text window rect would require more
	// invasive changes than this one, thus it's not really worth the effort
	// for a feature that was not present in the original game, and its
	// implementation is buggy in the first place.
	// Adjusting the restore rect properly fixes bug #3575276.

	if (wmprect.top > pwnd->dims.top) {
		pwnd->dims.moveTo(pwnd->dims.left, wmprect.top);
		if (restoreRect)
			pwnd->restoreRect.moveTo(pwnd->restoreRect.left, wmprect.top);
	}

	if (wmprect.bottom < pwnd->dims.bottom) {
		pwnd->dims.moveTo(pwnd->dims.left, wmprect.bottom - pwnd->dims.bottom + pwnd->dims.top);
		if (restoreRect)
			pwnd->restoreRect.moveTo(pwnd->restoreRect.left, wmprect.bottom - pwnd->restoreRect.bottom + pwnd->restoreRect.top);
	}

	if (wmprect.right < pwnd->dims.right) {
		pwnd->dims.moveTo(wmprect.right + pwnd->dims.left - pwnd->dims.right, pwnd->dims.top);
		if (restoreRect)
			pwnd->restoreRect.moveTo(wmprect.right + pwnd->restoreRect.left - pwnd->restoreRect.right, pwnd->restoreRect.top);
	}

	if (wmprect.left > pwnd->dims.left) {
		pwnd->dims.moveTo(wmprect.left, pwnd->dims.top);
		if (restoreRect)
			pwnd->restoreRect.moveTo(wmprect.left, pwnd->restoreRect.top);
	}

	pwnd->rect.moveTo(pwnd->rect.left + pwnd->dims.left - oldleft, pwnd->rect.top + pwnd->dims.top - oldtop);

	if (restoreRect == 0)
		pwnd->restoreRect = pwnd->dims;

	if (pwnd->restoreRect.top < 0 && g_sci->getPlatform() == Common::kPlatformMacintosh &&
		(style & SCI_WINDOWMGR_STYLE_USER) && _wmgrPort->top + pwnd->restoreRect.top >= 0) {
		// Special case for Dr. Brain 1 Mac (check above), applied to the
		// restore rectangle.
		pwnd->restoreRect.moveTo(pwnd->restoreRect.left, wmprect.top);
	}

	if (draw)
		drawWindow(pwnd);
	setPort((Port *)pwnd);

	// All SCI0 games till kq4 .502 (not including) did not adjust against _wmgrPort, we set _wmgrPort->top to 0 in that case
	setOrigin(pwnd->rect.left, pwnd->rect.top + _wmgrPort->top);
	pwnd->rect.moveTo(0, 0);
	return pwnd;
}

void GfxPorts::drawWindow(Window *pWnd) {
	if (pWnd->bDrawn)
		return;
	int16 wndStyle = pWnd->wndStyle;

	pWnd->bDrawn = true;
	Port *oldport = setPort(_wmgrPort);
	penColor(0);
	if ((wndStyle & SCI_WINDOWMGR_STYLE_TRANSPARENT) == 0) {
		pWnd->hSaved1 = _paint16->bitsSave(pWnd->restoreRect, GFX_SCREEN_MASK_VISUAL);
		if (pWnd->saveScreenMask & GFX_SCREEN_MASK_PRIORITY) {
			pWnd->hSaved2 = _paint16->bitsSave(pWnd->restoreRect, GFX_SCREEN_MASK_PRIORITY);
			if ((wndStyle & SCI_WINDOWMGR_STYLE_USER) == 0)
				_paint16->fillRect(pWnd->restoreRect, GFX_SCREEN_MASK_PRIORITY, 0, 15);
		}
	}

	// drawing frame,shadow and title
	if ((getSciVersion() >= SCI_VERSION_1_LATE) ? !(wndStyle & _styleUser) : wndStyle != _styleUser) {
		Common::Rect r = pWnd->dims;

		if (!(wndStyle & SCI_WINDOWMGR_STYLE_NOFRAME)) {
			r.top++;
			r.left++;
			_paint16->frameRect(r);// draw shadow
			r.translate(-1, -1);
			_paint16->frameRect(r);// draw actual window frame

			if (wndStyle & SCI_WINDOWMGR_STYLE_TITLE) {
				if (getSciVersion() <= SCI_VERSION_0_LATE) {
					// draw a black line between titlebar and actual window content for SCI0
					r.bottom = r.top + 10;
					_paint16->frameRect(r);
				}
				r.grow(-1);
				if (getSciVersion() <= SCI_VERSION_0_LATE)
					_paint16->fillRect(r, GFX_SCREEN_MASK_VISUAL, 8); // grey titlebar for SCI0
				else
					_paint16->fillRect(r, GFX_SCREEN_MASK_VISUAL, 0); // black titlebar for SCI01+
				if (!pWnd->title.empty()) {
					int16 oldcolor = getPort()->penClr;
					penColor(_screen->getColorWhite());
					_text16->Box(pWnd->title.c_str(), true, r, SCI_TEXT16_ALIGNMENT_CENTER, 0);
					penColor(oldcolor);
				}

				r.grow(+1);
				r.bottom = pWnd->dims.bottom - 1;
				r.top += 9;
			}

			r.grow(-1);
		}

		if (!(wndStyle & SCI_WINDOWMGR_STYLE_TRANSPARENT))
			_paint16->fillRect(r, GFX_SCREEN_MASK_VISUAL, pWnd->backClr);

		_paint16->bitsShow(pWnd->dims);
	}
	setPort(oldport);
}

void GfxPorts::removeWindow(Window *pWnd, bool reanimate) {
	setPort(_wmgrPort);
	_paint16->bitsRestore(pWnd->hSaved1);
	pWnd->hSaved1 = NULL_REG;
	_paint16->bitsRestore(pWnd->hSaved2);
	pWnd->hSaved2 = NULL_REG;
	if (!reanimate)
		_paint16->bitsShow(pWnd->restoreRect);
	else
		_paint16->kernelGraphRedrawBox(pWnd->restoreRect);
	_windowList.remove(pWnd);
	setPort(_windowList.back());
	// We will actually free this window after 15 kSetPort-calls
	// Sierra sci freed the pointer immediately, but pointer to that port
	//  still worked till the memory got overwritten. Some games depend
	//  on this (dispose a window and then kSetPort to it again for once)
	//  Those are actually script bugs, but patching all of those out
	//  would be quite a hassle and this just keeps compatibility
	//  (examples: hoyle 4 game menu and sq4cd inventory)
	//  sq4cd gum wrapper requires more than 10
	pWnd->counterTillFree = 15;
	_freeCounter++;
}

void GfxPorts::freeWindow(Window *pWnd) {
	if (!pWnd->hSaved1.isNull())
		_segMan->freeHunkEntry(pWnd->hSaved1);
	if (!pWnd->hSaved2.isNull())
		_segMan->freeHunkEntry(pWnd->hSaved1);
	_windowsById[pWnd->id] = NULL;
	delete pWnd;
}

void GfxPorts::updateWindow(Window *wnd) {
	reg_t handle;

	if (wnd->saveScreenMask && wnd->bDrawn) {
		handle = _paint16->bitsSave(wnd->restoreRect, GFX_SCREEN_MASK_VISUAL);
		_paint16->bitsRestore(wnd->hSaved1);
		wnd->hSaved1 = handle;
		if (wnd->saveScreenMask & GFX_SCREEN_MASK_PRIORITY) {
			handle = _paint16->bitsSave(wnd->restoreRect, GFX_SCREEN_MASK_PRIORITY);
			_paint16->bitsRestore(wnd->hSaved2);
			wnd->hSaved2 = handle;
		}
	}
}

Port *GfxPorts::getPortById(uint16 id) {
	return (id < _windowsById.size()) ? _windowsById[id] : NULL;
}

Port *GfxPorts::setPort(Port *newPort) {
	Port *oldPort = _curPort;
	_curPort = newPort;
	return oldPort;
}

Port *GfxPorts::getPort() {
	return _curPort;
}

void GfxPorts::setOrigin(int16 left, int16 top) {
	_curPort->left = left;
	_curPort->top = top;
}

void GfxPorts::moveTo(int16 left, int16 top) {
	_curPort->curTop = top;
	_curPort->curLeft = left;
}

void GfxPorts::move(int16 left, int16 top) {
	_curPort->curTop += top;
	_curPort->curLeft += left;
}

void GfxPorts::openPort(Port *port) {
	port->fontId = 0;
	port->fontHeight = 8;

	Port *tmp = _curPort;
	_curPort = port;
	_text16->SetFont(port->fontId);
	_curPort = tmp;

	port->top = 0;
	port->left = 0;
	port->greyedOutput = false;
	port->penClr = 0;
	port->backClr = _screen->getColorWhite();
	port->penMode = 0;
	port->rect = _bounds;
}

void GfxPorts::penColor(int16 color) {
	_curPort->penClr = color;
}

void GfxPorts::backColor(int16 color) {
	_curPort->backClr = color;
}

void GfxPorts::penMode(int16 mode) {
	_curPort->penMode = mode;
}

void GfxPorts::textGreyedOutput(bool state) {
	_curPort->greyedOutput = state;
}

int16 GfxPorts::getPointSize() {
	return _curPort->fontHeight;
}

void GfxPorts::offsetRect(Common::Rect &r) {
	r.top += _curPort->top;
	r.bottom += _curPort->top;
	r.left += _curPort->left;
	r.right += _curPort->left;
}

void GfxPorts::offsetLine(Common::Point &start, Common::Point &end) {
	start.x += _curPort->left;
	start.y += _curPort->top;
	end.x += _curPort->left;
	end.y += _curPort->top;
}

void GfxPorts::clipLine(Common::Point &start, Common::Point &end) {
	start.y = CLIP<int16>(start.y, _curPort->rect.top, _curPort->rect.bottom - 1);
	start.x = CLIP<int16>(start.x, _curPort->rect.left, _curPort->rect.right - 1);
	end.y = CLIP<int16>(end.y, _curPort->rect.top, _curPort->rect.bottom - 1);
	end.x = CLIP<int16>(end.x, _curPort->rect.left, _curPort->rect.right - 1);
}

void GfxPorts::priorityBandsInit(int16 bandCount, int16 top, int16 bottom) {
	int16 y;
	int32 bandSize;

	if (bandCount != -1)
		_priorityBandCount = bandCount;

	_priorityTop = top;
	_priorityBottom = bottom;

	// Do NOT modify this algo or optimize it anyhow, sierra sci used int32 for calculating the
	//  priority bands and by using double or anything rounding WILL destroy the result
	bandSize = ((_priorityBottom - _priorityTop) * 2000) / _priorityBandCount;

	memset(_priorityBands, 0, sizeof(byte) * _priorityTop);
	for (y = _priorityTop; y < _priorityBottom; y++)
		_priorityBands[y] = 1 + (((y - _priorityTop) * 2000) / bandSize);
	if (_priorityBandCount == 15) {
		// When having 15 priority bands, we actually replace band 15 with band 14, cause the original sci interpreter also
		//  does it that way as well
		y = _priorityBottom;
		while (_priorityBands[--y] == _priorityBandCount)
			_priorityBands[y]--;
	}
	// We fill space that is left over with the highest band (hardcoded 200 limit, because this algo isnt meant to be used on hires)
	for (y = _priorityBottom; y < 200; y++)
		_priorityBands[y] = _priorityBandCount;

	// adjust, if bottom is 200 (one over the actual screen range) - we could otherwise go possible out of bounds
	//  sierra sci also adjust accordingly
	if (_priorityBottom == 200)
		_priorityBottom--;
}

void GfxPorts::priorityBandsInit(byte *data) {
	int i = 0, inx;
	byte priority = 0;

	for (inx = 0; inx < 14; inx++) {
		priority = *data++;
		while (i < priority)
			_priorityBands[i++] = inx;
	}
	while (i < 200)
		_priorityBands[i++] = inx;
}

// Gets used to read priority bands data from sci1.1 pictures
void GfxPorts::priorityBandsInitSci11(byte *data) {
	byte priorityBands[14];
	for (int bandNo = 0; bandNo < 14; bandNo++) {
		priorityBands[bandNo] = READ_LE_UINT16(data);
		data += 2;
	}
	priorityBandsInit(priorityBands);
}

void GfxPorts::kernelInitPriorityBands() {
	if (_usesOldGfxFunctions) {
		priorityBandsInit(15, 42, 200);
	} else {
		if (getSciVersion() >= SCI_VERSION_1_1)
			priorityBandsInit(14, 0, 190);
		else
			priorityBandsInit(14, 42, 190);
	}
}

void GfxPorts::kernelGraphAdjustPriority(int top, int bottom) {
	if (_usesOldGfxFunctions) {
		priorityBandsInit(15, top, bottom);
	} else {
		priorityBandsInit(14, top, bottom);
	}
}

byte GfxPorts::kernelCoordinateToPriority(int16 y) {
	if (y < _priorityTop)
		return _priorityBands[_priorityTop];
	if (y > _priorityBottom)
		return _priorityBands[_priorityBottom];
	return _priorityBands[y];
}

int16 GfxPorts::kernelPriorityToCoordinate(byte priority) {
	int16 y;
	if (priority <= _priorityBandCount) {
		for (y = 0; y <= _priorityBottom; y++)
			if (_priorityBands[y] == priority)
				return y;
	}
	return _priorityBottom;
}

void GfxPorts::processEngineHunkList(WorklistManager &wm) {
	for (PortList::const_iterator it = _windowList.begin(); it != _windowList.end(); ++it) {
		if ((*it)->isWindow()) {
			Window *wnd = ((Window *)*it);
			wm.push(wnd->hSaved1);
			wm.push(wnd->hSaved2);
		}
	}
}

void GfxPorts::printWindowList(Console *con) {
	for (PortList::const_iterator it = _windowList.begin(); it != _windowList.end(); ++it) {
		if ((*it)->isWindow()) {
			Window *wnd = ((Window *)*it);
			con->DebugPrintf("%d: '%s' at %d, %d, (%d, %d, %d, %d), drawn: %d, style: %d\n",
					wnd->id, wnd->title.c_str(), wnd->left, wnd->top,
					wnd->rect.left, wnd->rect.top, wnd->rect.right, wnd->rect.bottom,
					wnd->bDrawn, wnd->wndStyle);
		}
	}
}

} // End of namespace Sci
