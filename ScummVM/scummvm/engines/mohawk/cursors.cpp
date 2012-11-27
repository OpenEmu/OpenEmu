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

#include "mohawk/cursors.h"
#include "mohawk/mohawk.h"
#include "mohawk/resource.h"

#include "common/macresman.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/winexe_ne.h"
#include "common/winexe_pe.h"
#include "graphics/cursorman.h"
#include "graphics/maccursor.h"
#include "graphics/wincursor.h"

#ifdef ENABLE_MYST
#include "mohawk/bitmap.h"
#include "mohawk/myst.h"
#endif

namespace Mohawk {

void CursorManager::showCursor() {
	CursorMan.showMouse(true);
}

void CursorManager::hideCursor() {
	CursorMan.showMouse(false);
}

void CursorManager::setDefaultCursor() {
	Graphics::Cursor *cursor = Graphics::makeDefaultWinCursor();

	CursorMan.replaceCursor(cursor->getSurface(), cursor->getWidth(), cursor->getHeight(), cursor->getHotspotX(),
			cursor->getHotspotY(), cursor->getKeyColor());
	CursorMan.replaceCursorPalette(cursor->getPalette(), cursor->getPaletteStartIndex(), cursor->getPaletteCount());

	delete cursor;
}

void CursorManager::setCursor(uint16 id) {
	// For the base class, just use the default cursor always
	setDefaultCursor();
}

void CursorManager::setMacCursor(Common::SeekableReadStream *stream) {
	assert(stream);

	Graphics::MacCursor *macCursor = new Graphics::MacCursor();

	if (!macCursor->readFromStream(*stream))
		error("Could not parse Mac cursor");

	CursorMan.replaceCursor(macCursor->getSurface(), macCursor->getWidth(), macCursor->getHeight(),
			macCursor->getHotspotX(), macCursor->getHotspotY(), macCursor->getKeyColor());
	CursorMan.replaceCursorPalette(macCursor->getPalette(), 0, 256);

	delete macCursor;
	delete stream;
}

void DefaultCursorManager::setCursor(uint16 id) {
	setMacCursor(_vm->getResource(_tag, id));
}

#ifdef ENABLE_MYST

MystCursorManager::MystCursorManager(MohawkEngine_Myst *vm) : _vm(vm) {
	_bmpDecoder = new MystBitmap();
}

MystCursorManager::~MystCursorManager() {
	delete _bmpDecoder;
}

void MystCursorManager::showCursor() {
	CursorMan.showMouse(true);
	_vm->_needsUpdate = true;
}

void MystCursorManager::hideCursor() {
	CursorMan.showMouse(false);
	_vm->_needsUpdate = true;
}

void MystCursorManager::setCursor(uint16 id) {
	// Zero means empty cursor
	if (id == 0) {
		static const byte emptyCursor = 0;
		CursorMan.replaceCursor(&emptyCursor, 1, 1, 0, 0, 0);
		return;
	}

	// Both Myst and Myst ME use the "MystBitmap" format for cursor images.
	MohawkSurface *mhkSurface = _bmpDecoder->decodeImage(_vm->getResource(ID_WDIB, id));
	Graphics::Surface *surface = mhkSurface->getSurface();
	Common::SeekableReadStream *clrcStream = _vm->getResource(ID_CLRC, id);
	uint16 hotspotX = clrcStream->readUint16LE();
	uint16 hotspotY = clrcStream->readUint16LE();
	delete clrcStream;

	// Myst ME stores some cursors as 24bpp images instead of 8bpp
	if (surface->format.bytesPerPixel == 1) {
		CursorMan.replaceCursor(surface->pixels, surface->w, surface->h, hotspotX, hotspotY, 0);
		CursorMan.replaceCursorPalette(mhkSurface->getPalette(), 0, 256);
	} else {
		Graphics::PixelFormat pixelFormat = g_system->getScreenFormat();
		CursorMan.replaceCursor(surface->pixels, surface->w, surface->h, hotspotX, hotspotY, pixelFormat.RGBToColor(255, 255, 255), false, &pixelFormat);
	}

	_vm->_needsUpdate = true;
	delete mhkSurface;
}

void MystCursorManager::setDefaultCursor() {
	setCursor(kDefaultMystCursor);
}

#endif

NECursorManager::NECursorManager(const Common::String &appName) {
	_exe = new Common::NEResources();

	if (!_exe->loadFromEXE(appName)) {
		// Not all have cursors anyway, so this is not a problem
		delete _exe;
		_exe = 0;
	}
}

NECursorManager::~NECursorManager() {
	delete _exe;
}

void NECursorManager::setCursor(uint16 id) {
	if (_exe) {
		Graphics::WinCursorGroup *cursorGroup = Graphics::WinCursorGroup::createCursorGroup(*_exe, id);

		if (cursorGroup) {
			Graphics::Cursor *cursor = cursorGroup->cursors[0].cursor;
			CursorMan.replaceCursor(cursor->getSurface(), cursor->getWidth(), cursor->getHeight(), cursor->getHotspotX(), cursor->getHotspotY(), cursor->getKeyColor());
			CursorMan.replaceCursorPalette(cursor->getPalette(), 0, 256);
			return;
		}
	}

	// Last resort (not all have cursors)
	setDefaultCursor();
}

MacCursorManager::MacCursorManager(const Common::String &appName) {
	if (!appName.empty()) {
		_resFork = new Common::MacResManager();

		if (!_resFork->open(appName)) {
			// Not all have cursors anyway, so this is not a problem
			delete _resFork;
			_resFork = 0;
		}
	} else {
		_resFork = 0;
	}
}

MacCursorManager::~MacCursorManager() {
	delete _resFork;
}

void MacCursorManager::setCursor(uint16 id) {
	if (!_resFork) {
		setDefaultCursor();
		return;
	}

	// Try a color cursor first
	Common::SeekableReadStream *stream = _resFork->getResource(MKTAG('c','r','s','r'), id);

	// Fall back to monochrome cursors
	if (!stream)
		stream = _resFork->getResource(MKTAG('C','U','R','S'), id);

	if (stream)
		setMacCursor(stream);
	else
		setDefaultCursor();
}

LivingBooksCursorManager_v2::LivingBooksCursorManager_v2() {
	// Try to open the system archive if we have it
	_sysArchive = new MohawkArchive();

	if (!_sysArchive->openFile("system.mhk")) {
		delete _sysArchive;
		_sysArchive = 0;
	}
}

LivingBooksCursorManager_v2::~LivingBooksCursorManager_v2() {
	delete _sysArchive;
}

void LivingBooksCursorManager_v2::setCursor(uint16 id) {
	if (_sysArchive && _sysArchive->hasResource(ID_TCUR, id)) {
		setMacCursor(_sysArchive->getResource(ID_TCUR, id));
	} else {
		// TODO: Handle generated cursors
	}
}

void LivingBooksCursorManager_v2::setCursor(const Common::String &name) {
	if (!_sysArchive)
		return;

	uint16 id = _sysArchive->findResourceID(ID_TCUR, name);
	if (id == 0xffff)
		error("Could not find cursor '%s'", name.c_str());
	else
		setCursor(id);
}

PECursorManager::PECursorManager(const Common::String &appName) {
	_exe = new Common::PEResources();

	if (!_exe->loadFromEXE(appName)) {
		// Not all have cursors anyway, so this is not a problem
		delete _exe;
		_exe = 0;
	}
}

PECursorManager::~PECursorManager() {
	delete _exe;
}

void PECursorManager::setCursor(uint16 id) {
	if (_exe) {
		Graphics::WinCursorGroup *cursorGroup = Graphics::WinCursorGroup::createCursorGroup(*_exe, id);

		if (cursorGroup) {
			Graphics::Cursor *cursor = cursorGroup->cursors[0].cursor;
			CursorMan.replaceCursor(cursor->getSurface(), cursor->getWidth(), cursor->getHeight(), cursor->getHotspotX(), cursor->getHotspotY(), cursor->getKeyColor());
			CursorMan.replaceCursorPalette(cursor->getPalette(), 0, 256);
			delete cursorGroup;
			return;
		}
	}

	// Last resort (not all have cursors)
	setDefaultCursor();
}

} // End of namespace Mohawk
