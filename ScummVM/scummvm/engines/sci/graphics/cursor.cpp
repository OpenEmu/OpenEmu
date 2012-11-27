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

#include "common/config-manager.h"
#include "common/events.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/cursorman.h"
#include "graphics/maccursor.h"

#include "sci/sci.h"
#include "sci/event.h"
#include "sci/engine/state.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/view.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/maciconbar.h"

namespace Sci {

GfxCursor::GfxCursor(ResourceManager *resMan, GfxPalette *palette, GfxScreen *screen)
	: _resMan(resMan), _palette(palette), _screen(screen) {

	_upscaledHires = _screen->getUpscaledHires();
	_isVisible = true;

	// center mouse cursor
	setPosition(Common::Point(_screen->getWidth() / 2, _screen->getHeight() / 2));
	_moveZoneActive = false;

	_zoomZoneActive = false;
	_zoomZone = Common::Rect();
	_zoomCursorView = 0;
	_zoomCursorLoop = 0;
	_zoomCursorCel = 0;
	_zoomPicView = 0;
	_zoomColor = 0;
	_zoomMultiplier = 0;
	_cursorSurface = 0;

	if (g_sci && g_sci->getGameId() == GID_KQ6 && g_sci->getPlatform() == Common::kPlatformWindows)
		_useOriginalKQ6WinCursors = ConfMan.getBool("windows_cursors");
	else
		_useOriginalKQ6WinCursors = false;

	if (g_sci && g_sci->getGameId() == GID_SQ4 && getSciVersion() == SCI_VERSION_1_1)
		_useSilverSQ4CDCursors = ConfMan.getBool("silver_cursors");
	else
		_useSilverSQ4CDCursors = false;
}

GfxCursor::~GfxCursor() {
	purgeCache();
	kernelClearZoomZone();
}

void GfxCursor::init(GfxCoordAdjuster *coordAdjuster, EventManager *event) {
	_coordAdjuster = coordAdjuster;
	_event = event;
}

void GfxCursor::kernelShow() {
	CursorMan.showMouse(true);
	_isVisible = true;
}

void GfxCursor::kernelHide() {
	CursorMan.showMouse(false);
	_isVisible = false;
}

bool GfxCursor::isVisible() {
	return _isVisible;
}

void GfxCursor::purgeCache() {
	for (CursorCache::iterator iter = _cachedCursors.begin(); iter != _cachedCursors.end(); ++iter) {
		delete iter->_value;
		iter->_value = 0;
	}

	_cachedCursors.clear();
}

void GfxCursor::kernelSetShape(GuiResourceId resourceId) {
	Resource *resource;
	byte *resourceData;
	Common::Point hotspot = Common::Point(0, 0);
	byte colorMapping[4];
	int16 x, y;
	byte color;
	int16 maskA, maskB;
	byte *pOut;
	byte *rawBitmap = new byte[SCI_CURSOR_SCI0_HEIGHTWIDTH * SCI_CURSOR_SCI0_HEIGHTWIDTH];
	int16 heightWidth;

	if (resourceId == -1) {
		// no resourceId given, so we actually hide the cursor
		kernelHide();
		delete[] rawBitmap;
		return;
	}

	// Load cursor resource...
	resource = _resMan->findResource(ResourceId(kResourceTypeCursor, resourceId), false);
	if (!resource)
		error("cursor resource %d not found", resourceId);
	if (resource->size != SCI_CURSOR_SCI0_RESOURCESIZE)
		error("cursor resource %d has invalid size", resourceId);

	resourceData = resource->data;

	if (getSciVersion() <= SCI_VERSION_01) {
		// SCI0 cursors contain hotspot flags, not actual hotspot coordinates.
		// If bit 0 of resourceData[3] is set, the hotspot should be centered,
		// otherwise it's in the top left of the mouse cursor.
		hotspot.x = hotspot.y = resourceData[3] ? SCI_CURSOR_SCI0_HEIGHTWIDTH / 2 : 0;
	} else {
		// Cursors in newer SCI versions contain actual hotspot coordinates.
		hotspot.x = READ_LE_UINT16(resourceData);
		hotspot.y = READ_LE_UINT16(resourceData + 2);
	}

	// Now find out what colors we are supposed to use
	colorMapping[0] = 0; // Black is hardcoded
	colorMapping[1] = _screen->getColorWhite(); // White is also hardcoded
	colorMapping[2] = SCI_CURSOR_SCI0_TRANSPARENCYCOLOR;
	colorMapping[3] = _palette->matchColor(170, 170, 170); // Grey
	// TODO: Figure out if the grey color is hardcoded
	// HACK for the magnifier cursor in LB1, fixes its color (bug #3487092)
	if (g_sci->getGameId() == GID_LAURABOW && resourceId == 1)
		colorMapping[3] = _screen->getColorWhite();
	// HACK for Longbow cursors, fixes the shade of grey they're using (bug #3489101)
	if (g_sci->getGameId() == GID_LONGBOW)
		colorMapping[3] = _palette->matchColor(223, 223, 223); // Light Grey

	// Seek to actual data
	resourceData += 4;

	pOut = rawBitmap;
	for (y = 0; y < SCI_CURSOR_SCI0_HEIGHTWIDTH; y++) {
		maskA = READ_LE_UINT16(resourceData + (y << 1));
		maskB = READ_LE_UINT16(resourceData + 32 + (y << 1));

		for (x = 0; x < SCI_CURSOR_SCI0_HEIGHTWIDTH; x++) {
			color = (((maskA << x) & 0x8000) | (((maskB << x) >> 1) & 0x4000)) >> 14;
			*pOut++ = colorMapping[color];
		}
	}

	heightWidth = SCI_CURSOR_SCI0_HEIGHTWIDTH;

	if (_upscaledHires) {
		// Scale cursor by 2x - note: sierra didn't do this, but it looks much better
		heightWidth *= 2;
		hotspot.x *= 2;
		hotspot.y *= 2;
		byte *upscaledBitmap = new byte[heightWidth * heightWidth];
		_screen->scale2x(rawBitmap, upscaledBitmap, SCI_CURSOR_SCI0_HEIGHTWIDTH, SCI_CURSOR_SCI0_HEIGHTWIDTH);
		delete[] rawBitmap;
		rawBitmap = upscaledBitmap;
	}

	if (hotspot.x >= heightWidth || hotspot.y >= heightWidth) {
		error("cursor %d's hotspot (%d, %d) is out of range of the cursor's dimensions (%dx%d)",
				resourceId, hotspot.x, hotspot.y, heightWidth, heightWidth);
	}

	CursorMan.replaceCursor(rawBitmap, heightWidth, heightWidth, hotspot.x, hotspot.y, SCI_CURSOR_SCI0_TRANSPARENCYCOLOR);
	kernelShow();

	delete[] rawBitmap;
}

void GfxCursor::kernelSetView(GuiResourceId viewNum, int loopNum, int celNum, Common::Point *hotspot) {
	if (_cachedCursors.size() >= MAX_CACHED_CURSORS)
		purgeCache();

	// Use the original Windows cursors in KQ6, if requested
	if (_useOriginalKQ6WinCursors)
		viewNum += 2000;		// Windows cursors

	if (g_sci->getGameId() == GID_PHANTASMAGORIA2) {
		// HACK: Ignore cursor views for Phantasmagoria 2. They've got
		// differences from other SCI32 views, thus we skip them for
		// now, otherwise our view decoding code will crash.
		// The view code will crash with *any* view in P2, but this hack
		// allows the game to start and show the menu.
		// TODO: Remove once the view code is updated to handle
		// Phantasmagoria 2 views.
		warning("TODO: Cursor views for Phantasmagoria 2");
		return;
	}

	// Use the alternate silver cursors in SQ4 CD, if requested
	if (_useSilverSQ4CDCursors) {
		switch(viewNum) {
		case 850:
		case 852:
		case 854:
		case 856:
			celNum = 3;
			break;
		case 851:
		case 853:
		case 855:
		case 999:
			celNum = 2;
			break;
		default:
			break;
		}
	}

	if (!_cachedCursors.contains(viewNum))
		_cachedCursors[viewNum] = new GfxView(_resMan, _screen, _palette, viewNum);

	GfxView *cursorView = _cachedCursors[viewNum];

	const CelInfo *celInfo = cursorView->getCelInfo(loopNum, celNum);
	int16 width = celInfo->width;
	int16 height = celInfo->height;
	byte clearKey = celInfo->clearKey;
	Common::Point *cursorHotspot = hotspot;

	if (!cursorHotspot)
		// Compute hotspot from xoffset/yoffset
		cursorHotspot = new Common::Point((celInfo->width >> 1) - celInfo->displaceX, celInfo->height - celInfo->displaceY - 1);

	// Eco Quest 1 uses a 1x1 transparent cursor to hide the cursor from the
	// user. Some scalers don't seem to support this
	if (width < 2 || height < 2) {
		kernelHide();
		delete cursorHotspot;
		return;
	}

	const byte *rawBitmap = cursorView->getBitmap(loopNum, celNum);
	if (_upscaledHires && !_useOriginalKQ6WinCursors) {
		// Scale cursor by 2x - note: sierra didn't do this, but it looks much better
		width *= 2;
		height *= 2;
		cursorHotspot->x *= 2;
		cursorHotspot->y *= 2;
		byte *cursorBitmap = new byte[width * height];
		_screen->scale2x(rawBitmap, cursorBitmap, celInfo->width, celInfo->height);
		CursorMan.replaceCursor(cursorBitmap, width, height, cursorHotspot->x, cursorHotspot->y, clearKey);
		delete[] cursorBitmap;
	} else {
		CursorMan.replaceCursor(rawBitmap, width, height, cursorHotspot->x, cursorHotspot->y, clearKey);
	}

	kernelShow();

	delete cursorHotspot;
}

// this list contains all mandatory set cursor changes, that need special handling
//  ffs. GfxCursor::setPosition (below)
//    Game,            newPosition, validRect
static const SciCursorSetPositionWorkarounds setPositionWorkarounds[] = {
	{ GID_ISLANDBRAIN, 84, 109,     46, 76, 174, 243 }, // island of dr. brain / game menu
	{ GID_ISLANDBRAIN,143, 135,     57, 102, 163, 218 },// island of dr. brain / pause menu within copy protection
	{ GID_LSL5,        23, 171,     0, 0, 26, 320 },    // larry 5 / skip forward helper
	{ GID_QFG1VGA,     64, 174,     40, 37, 74, 284 },  // Quest For Glory 1 VGA / run/walk/sleep sub-menu
	{ GID_QFG3,        70, 170,     40, 61, 81, 258 },  // Quest For Glory 3 / run/walk/sleep sub-menu
	{ (SciGameId)0,    -1, -1,     -1, -1, -1, -1 }
};

void GfxCursor::setPosition(Common::Point pos) {
	// Don't set position, when cursor is not visible.
	// This fixes eco quest 1 (floppy) right at the start, which is setting
	// mouse cursor to (0,0) all the time during the intro. It's escapeable
	// (now) by moving to the left or top, but it's getting on your nerves. This
	// could theoretically break some things, although sierra normally sets
	// position only when showing the cursor.
	if (!_isVisible)
		return;

	if (!_upscaledHires) {
		g_system->warpMouse(pos.x, pos.y);
	} else {
		_screen->adjustToUpscaledCoordinates(pos.y, pos.x);
		g_system->warpMouse(pos.x, pos.y);
	}

	// Some games display a new menu, set mouse position somewhere within and
	//  expect it to be in there. This is fine for a real mouse, but on wii using
	//  wii-mote or touch interfaces this won't work. In fact on those platforms
	//  the menus will close immediately because of that behavior.
	// We identify those cases and set a reaction-rect. If the mouse it outside
	//  of that rect, we won't report the position back to the scripts.
	//  As soon as the mouse was inside once, we will revert to normal behavior
	// Currently this code is enabled for all platforms, especially because we can't
	//  differentiate between e.g. Windows used via mouse and Windows used via touchscreen
	// The workaround won't hurt real-mouse platforms
	const SciGameId gameId = g_sci->getGameId();
	const SciCursorSetPositionWorkarounds *workaround;
	workaround = setPositionWorkarounds;
	while (workaround->newPositionX != -1) {
		if (workaround->gameId == gameId
			&& ((workaround->newPositionX == pos.x) && (workaround->newPositionY == pos.y))) {
			EngineState *s = g_sci->getEngineState();
			s->_cursorWorkaroundActive = true;
			s->_cursorWorkaroundPoint = pos;
			s->_cursorWorkaroundRect = Common::Rect(workaround->rectLeft, workaround->rectTop, workaround->rectRight, workaround->rectBottom);
			return;
		}
		workaround++;
	}
}

Common::Point GfxCursor::getPosition() {
	Common::Point mousePos = g_system->getEventManager()->getMousePos();

	if (_upscaledHires)
		_screen->adjustBackUpscaledCoordinates(mousePos.y, mousePos.x);

	return mousePos;
}

void GfxCursor::refreshPosition() {
	Common::Point mousePoint = getPosition();

	if (_moveZoneActive) {
		bool clipped = false;

		if (mousePoint.x < _moveZone.left) {
			mousePoint.x = _moveZone.left;
			clipped = true;
		} else if (mousePoint.x >= _moveZone.right) {
			mousePoint.x = _moveZone.right - 1;
			clipped = true;
		}

		if (mousePoint.y < _moveZone.top) {
			mousePoint.y = _moveZone.top;
			clipped = true;
		} else if (mousePoint.y >= _moveZone.bottom) {
			mousePoint.y = _moveZone.bottom - 1;
			clipped = true;
		}

		// FIXME: Do this only when mouse is grabbed?
		if (clipped)
			setPosition(mousePoint);
	}

	if (_zoomZoneActive) {
		// Cursor
		const CelInfo *cursorCelInfo = _zoomCursorView->getCelInfo(_zoomCursorLoop, _zoomCursorCel);
		const byte *cursorBitmap = _zoomCursorView->getBitmap(_zoomCursorLoop, _zoomCursorCel);
		// Pic
		const CelInfo *picCelInfo = _zoomPicView->getCelInfo(0, 0);
		const byte *rawPicBitmap = _zoomPicView->getBitmap(0, 0);

		// Compute hotspot of cursor
		Common::Point cursorHotspot = Common::Point((cursorCelInfo->width >> 1) - cursorCelInfo->displaceX, cursorCelInfo->height - cursorCelInfo->displaceY - 1);

		int16 targetX = ((mousePoint.x - _moveZone.left) * _zoomMultiplier);
		int16 targetY = ((mousePoint.y - _moveZone.top) * _zoomMultiplier);
		if (targetX < 0)
			targetX = 0;
		if (targetY < 0)
			targetY = 0;

		targetX -= cursorHotspot.x;
		targetY -= cursorHotspot.y;

		// Sierra SCI actually drew only within zoom area, thus removing the need to fill any other pixels with upmost/left
		//  color of the picture cel. This also made the cursor not appear on top of everything. They actually drew the
		//  cursor manually within kAnimate processing and used a hidden cursor for moving.
		//  TODO: we should also do this

		// Replace the special magnifier color with the associated magnified pixels
		for (int x = 0; x < cursorCelInfo->width; x++) {
			for (int y = 0; y < cursorCelInfo->height; y++) {
				int curPos = cursorCelInfo->width * y + x;
				if (cursorBitmap[curPos] == _zoomColor) {
					int16 rawY = targetY + y;
					int16 rawX = targetX + x;
					if ((rawY >= 0) && (rawY < picCelInfo->height) && (rawX >= 0) && (rawX < picCelInfo->width)) {
						int rawPos = picCelInfo->width * rawY + rawX;
						_cursorSurface[curPos] = rawPicBitmap[rawPos];
					} else {
						_cursorSurface[curPos] = rawPicBitmap[0]; // use left and upmost pixel color
					}
				}
			}
		}

		CursorMan.replaceCursor(_cursorSurface, cursorCelInfo->width, cursorCelInfo->height, cursorHotspot.x, cursorHotspot.y, cursorCelInfo->clearKey);
	}
}

void GfxCursor::kernelResetMoveZone() {
	_moveZoneActive = false;
}

void GfxCursor::kernelSetMoveZone(Common::Rect zone) {
	_moveZone = zone;
	_moveZoneActive = true;
}

void GfxCursor::kernelClearZoomZone() {
	kernelResetMoveZone();
	_zoomZone = Common::Rect();
	_zoomColor = 0;
	_zoomMultiplier = 0;
	_zoomZoneActive = false;
	delete _zoomCursorView;
	_zoomCursorView = 0;
	delete _zoomPicView;
	_zoomPicView = 0;
	delete[] _cursorSurface;
	_cursorSurface = 0;
}

void GfxCursor::kernelSetZoomZone(byte multiplier, Common::Rect zone, GuiResourceId viewNum, int loopNum, int celNum, GuiResourceId picNum, byte zoomColor) {
	kernelClearZoomZone();

	_zoomMultiplier = multiplier;

	if (_zoomMultiplier != 1 && _zoomMultiplier != 2 && _zoomMultiplier != 4)
		error("Unexpected zoom multiplier (expected 1, 2 or 4)");

	_zoomCursorView = new GfxView(_resMan, _screen, _palette, viewNum);
	_zoomCursorLoop = (byte)loopNum;
	_zoomCursorCel = (byte)celNum;
	_zoomPicView = new GfxView(_resMan, _screen, _palette, picNum);
	const CelInfo *cursorCelInfo = _zoomCursorView->getCelInfo(_zoomCursorLoop, _zoomCursorCel);
	const byte *cursorBitmap = _zoomCursorView->getBitmap(_zoomCursorLoop, _zoomCursorCel);
	_cursorSurface = new byte[cursorCelInfo->width * cursorCelInfo->height];
	memcpy(_cursorSurface, cursorBitmap, cursorCelInfo->width * cursorCelInfo->height);

	_zoomZone = zone;
	kernelSetMoveZone(_zoomZone);

	_zoomColor = zoomColor;
	_zoomZoneActive = true;
}

void GfxCursor::kernelSetPos(Common::Point pos) {
	_coordAdjuster->setCursorPos(pos);
	kernelMoveCursor(pos);
}

void GfxCursor::kernelMoveCursor(Common::Point pos) {
	_coordAdjuster->moveCursor(pos);
	if (pos.x > _screen->getWidth() || pos.y > _screen->getHeight()) {
		warning("attempt to place cursor at invalid coordinates (%d, %d)", pos.y, pos.x);
		return;
	}

	setPosition(pos);

	// Trigger event reading to make sure the mouse coordinates will
	// actually have changed the next time we read them.
	_event->getSciEvent(SCI_EVENT_PEEK);
}

void GfxCursor::kernelSetMacCursor(GuiResourceId viewNum, int loopNum, int celNum, Common::Point *hotspot) {
	// Here we try to map the view number onto the cursor. What they did was keep the
	// kSetCursor calls the same, but perform remapping on the cursors. They also took
	// it a step further and added a new kPlatform sub-subop that handles remapping
	// automatically. The view resources may exist, but none of the games actually
	// use them.

	if (_macCursorRemap.empty()) {
		// QFG1/Freddy/Hoyle4 use a straight viewNum->cursor ID mapping
		// KQ6 uses this mapping for its cursors
		if (g_sci->getGameId() == GID_KQ6) {
			if (viewNum == 990)      // Inventory Cursors
				viewNum = loopNum * 16 + celNum + 2000;
			else if (viewNum == 998) // Regular Cursors
				viewNum = celNum + 1000;
			else                     // Unknown cursor, ignored
				return;
		}
		if (g_sci->hasMacIconBar())
			g_sci->_gfxMacIconBar->setInventoryIcon(viewNum);
	} else {
		// If we do have the list, we'll be using a remap based on what the
		// scripts have given us.
		for (uint32 i = 0; i < _macCursorRemap.size(); i++) {
			if (viewNum == _macCursorRemap[i]) {
				viewNum = (i + 1) * 0x100 + loopNum * 0x10 + celNum;
				break;
			}

			if (i == _macCursorRemap.size())
				error("Unmatched Mac cursor %d", viewNum);
		}
	}

	Resource *resource = _resMan->findResource(ResourceId(kResourceTypeCursor, viewNum), false);

	if (!resource) {
		// The cursor resources often don't exist, this is normal behavior
		debug(0, "Mac cursor %d not found", viewNum);
		return;
	}

	CursorMan.disableCursorPalette(false);

	assert(resource);

	Common::MemoryReadStream resStream(resource->data, resource->size);
	Graphics::MacCursor *macCursor = new Graphics::MacCursor();

	if (!macCursor->readFromStream(resStream)) {
		warning("Failed to load Mac cursor %d", viewNum);
		return;
	}

	CursorMan.replaceCursor(macCursor->getSurface(), macCursor->getWidth(), macCursor->getHeight(),
			macCursor->getHotspotX(), macCursor->getHotspotY(), macCursor->getKeyColor());
	CursorMan.replaceCursorPalette(macCursor->getPalette(), 0, 256);

	delete macCursor;
	kernelShow();
}

void GfxCursor::setMacCursorRemapList(int cursorCount, reg_t *cursors) {
	for (int i = 0; i < cursorCount; i++)
		_macCursorRemap.push_back(cursors[i].toUint16());
}

} // End of namespace Sci
