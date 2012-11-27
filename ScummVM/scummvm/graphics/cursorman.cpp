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

#include "graphics/cursorman.h"

#include "common/system.h"
#include "common/stack.h"

namespace Common {
DECLARE_SINGLETON(Graphics::CursorManager);
}

namespace Graphics {

CursorManager::~CursorManager() {
	for (Common::Stack<Cursor *>::size_type i = 0; i < _cursorStack.size(); ++i)
		delete _cursorStack[i];
	_cursorStack.clear();
	for (Common::Stack<Palette *>::size_type i = 0; i < _cursorPaletteStack.size(); ++i)
		delete _cursorPaletteStack[i];
	_cursorPaletteStack.clear();
}

bool CursorManager::isVisible() {
	if (_cursorStack.empty())
		return false;
	return _cursorStack.top()->_visible;
}

bool CursorManager::showMouse(bool visible) {
	if (_cursorStack.empty())
		return false;

	_cursorStack.top()->_visible = visible;

	// Should work, even if there's just a dummy cursor on the stack.
	return g_system->showMouse(visible);
}

void CursorManager::pushCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format) {
	Cursor *cur = new Cursor(buf, w, h, hotspotX, hotspotY, keycolor, dontScale, format);

	cur->_visible = isVisible();
	_cursorStack.push(cur);

	if (buf) {
		g_system->setMouseCursor(cur->_data, w, h, hotspotX, hotspotY, keycolor, dontScale, format);
	}
}

void CursorManager::popCursor() {
	if (_cursorStack.empty())
		return;

	Cursor *cur = _cursorStack.pop();
	delete cur;

	if (!_cursorStack.empty()) {
		cur = _cursorStack.top();
		g_system->setMouseCursor(cur->_data, cur->_width, cur->_height, cur->_hotspotX, cur->_hotspotY, cur->_keycolor, cur->_dontScale, &cur->_format);
	}

	g_system->showMouse(isVisible());
}


void CursorManager::popAllCursors() {
	while (!_cursorStack.empty()) {
		Cursor *cur = _cursorStack.pop();
		delete cur;
	}

	if (g_system->hasFeature(OSystem::kFeatureCursorPalette)) {
		while (!_cursorPaletteStack.empty()) {
			Palette *pal = _cursorPaletteStack.pop();
			delete pal;
		}
	}

	g_system->showMouse(isVisible());
}

void CursorManager::replaceCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format) {

	if (_cursorStack.empty()) {
		pushCursor(buf, w, h, hotspotX, hotspotY, keycolor, dontScale, format);
		return;
	}

	Cursor *cur = _cursorStack.top();

#ifdef USE_RGB_COLOR
	uint size;
	if (!format)
		size = w * h;
	else
		size = w * h * format->bytesPerPixel;
#else
	uint size = w * h;
#endif

	if (cur->_size < size) {
		delete[] cur->_data;
		cur->_data = new byte[size];
		cur->_size = size;
	}

	if (buf && cur->_data)
		memcpy(cur->_data, buf, size);

	cur->_width = w;
	cur->_height = h;
	cur->_hotspotX = hotspotX;
	cur->_hotspotY = hotspotY;
	cur->_keycolor = keycolor;
	cur->_dontScale = dontScale;
#ifdef USE_RGB_COLOR
	if (format)
		cur->_format = *format;
	else
		cur->_format = Graphics::PixelFormat::createFormatCLUT8();
#endif

	g_system->setMouseCursor(cur->_data, w, h, hotspotX, hotspotY, keycolor, dontScale, format);
}

bool CursorManager::supportsCursorPalettes() {
	return g_system->hasFeature(OSystem::kFeatureCursorPalette);
}

void CursorManager::disableCursorPalette(bool disable) {
	if (!g_system->hasFeature(OSystem::kFeatureCursorPalette))
		return;

	if (_cursorPaletteStack.empty())
		return;

	Palette *pal = _cursorPaletteStack.top();
	pal->_disabled = disable;

	g_system->setFeatureState(OSystem::kFeatureCursorPalette, !disable);
}

void CursorManager::pushCursorPalette(const byte *colors, uint start, uint num) {
	if (!g_system->hasFeature(OSystem::kFeatureCursorPalette))
		return;

	Palette *pal = new Palette(colors, start, num);
	_cursorPaletteStack.push(pal);

	if (num)
		g_system->setCursorPalette(colors, start, num);
	else
		g_system->setFeatureState(OSystem::kFeatureCursorPalette, false);
}

void CursorManager::popCursorPalette() {
	if (!g_system->hasFeature(OSystem::kFeatureCursorPalette))
		return;

	if (_cursorPaletteStack.empty())
		return;

	Palette *pal = _cursorPaletteStack.pop();
	delete pal;

	if (_cursorPaletteStack.empty()) {
		g_system->setFeatureState(OSystem::kFeatureCursorPalette, false);
		return;
	}

	pal = _cursorPaletteStack.top();

	if (pal->_num && !pal->_disabled)
		g_system->setCursorPalette(pal->_data, pal->_start, pal->_num);
	else
		g_system->setFeatureState(OSystem::kFeatureCursorPalette, false);
}

void CursorManager::replaceCursorPalette(const byte *colors, uint start, uint num) {
	if (!g_system->hasFeature(OSystem::kFeatureCursorPalette))
		return;

	if (_cursorPaletteStack.empty()) {
		pushCursorPalette(colors, start, num);
		return;
	}

	Palette *pal = _cursorPaletteStack.top();
	uint size = 3 * num;

	if (pal->_size < size) {
		// Could not re-use the old buffer. Create a new one.
		delete[] pal->_data;
		pal->_data = new byte[size];
		pal->_size = size;
	}

	pal->_start = start;
	pal->_num = num;

	if (num) {
		memcpy(pal->_data, colors, size);
		g_system->setCursorPalette(pal->_data, pal->_start, pal->_num);
	} else {
		g_system->setFeatureState(OSystem::kFeatureCursorPalette, false);
	}
}

CursorManager::Cursor::Cursor(const void *data, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format) {
#ifdef USE_RGB_COLOR
	if (!format)
		_format = Graphics::PixelFormat::createFormatCLUT8();
	 else
		_format = *format;
	_size = w * h * _format.bytesPerPixel;
	_keycolor = keycolor & ((1 << (_format.bytesPerPixel << 3)) - 1);
#else
	_format = Graphics::PixelFormat::createFormatCLUT8();
	_size = w * h;
	_keycolor = keycolor & 0xFF;
#endif
	_data = new byte[_size];
	if (data && _data)
		memcpy(_data, data, _size);
	_width = w;
	_height = h;
	_hotspotX = hotspotX;
	_hotspotY = hotspotY;
	_dontScale = dontScale;
}

CursorManager::Cursor::~Cursor() {
	delete[] _data;
}

CursorManager::Palette::Palette(const byte *colors, uint start, uint num) {
	_start = start;
	_num = num;
	_size = 3 * num;

	if (num) {
		_data = new byte[_size];
		memcpy(_data, colors, _size);
	} else {
		_data = NULL;
	}

	_disabled = false;
}

CursorManager::Palette::~Palette() {
	delete[] _data;
}

} // End of namespace Graphics
