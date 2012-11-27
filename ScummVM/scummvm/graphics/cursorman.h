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

#ifndef GRAPHICS_CURSORMAN_H
#define GRAPHICS_CURSORMAN_H

#include "common/scummsys.h"
#include "common/stack.h"
#include "common/singleton.h"
#include "graphics/pixelformat.h"

namespace Graphics {

class CursorManager : public Common::Singleton<CursorManager> {
public:
	/** Query whether the mouse cursor is visible. */
	bool isVisible();

	/**
	 * Show or hide the mouse cursor.
	 *
	 * This function does not call OSystem::updateScreen, when visible is true.
	 * This fact might result in a non visible mouse cursor if the caller does
	 * not call OSystem::updateScreen itself after a showMouse(true) call.
	 *
	 * TODO: We might want to reconsider this behavior, it might be confusing
	 * for the user to call OSystem::updateScreen separately, on the other
	 * hand OSystem::updateScreen might as well display unwanted changes on
	 * the screen. Another alternative would be to let the backend worry
	 * about this on OSystem::showMouse call.
	 *
	 * @see OSystem::showMouse.
	 */
	bool showMouse(bool visible);

	/**
	 * Push a new cursor onto the stack, and set it in the backend. A local
	 * copy will be made of the cursor data, so the original buffer can be
	 * safely freed afterwards.
	 *
	 * @param buf		the new cursor data
	 * @param w			the width
	 * @param h			the height
	 * @param hotspotX	the hotspot X coordinate
	 * @param hotspotY	the hotspot Y coordinate
	 * @param keycolor	the color value for the transparent color. This may not exceed
	 *                  the maximum color value as defined by format.
	 * @param dontScale	Whether the cursor should never be scaled. An exception are high ppi displays, where the cursor
	 *                  would be too small to notice otherwise, these are allowed to scale the cursor anyway.
	 * @param format	a pointer to the pixel format which the cursor graphic uses,
	 *					CLUT8 will be used if this is NULL or not specified.
	 * @note It is ok for the buffer to be a NULL pointer. It is sometimes
	 *       useful to push a "dummy" cursor and modify it later. The
	 *       cursor will be added to the stack, but not to the backend.
	 */
	void pushCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL);

	/**
	 * Pop a cursor from the stack, and restore the previous one to the
	 * backend. If there is no previous cursor, the cursor is hidden.
	 */
	void popCursor();

	/**
	 * Replace the current cursor on the stack. If the stack is empty, the
	 * cursor is pushed instead. It's a slightly more optimized way of
	 * popping the old cursor before pushing the new one.
	 *
	 * @param buf		the new cursor data
	 * @param w		the width
	 * @param h		the height
	 * @param hotspotX	the hotspot X coordinate
	 * @param hotspotY	the hotspot Y coordinate
	 * @param keycolor	the color value for the transparent color. This may not exceed
	 *                  the maximum color value as defined by format.
	 * @param dontScale	Whether the cursor should never be scaled. An exception are high ppi displays, where the cursor
	 *                  would be too small to notice otherwise, these are allowed to scale the cursor anyway.
	 * @param format	a pointer to the pixel format which the cursor graphic uses,
	 *					CLUT8 will be used if this is NULL or not specified.
	 */
	void replaceCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL);

	/**
	 * Pop all of the cursors and cursor palettes from their respective stacks.
	 * The purpose is to ensure that all unecessary cursors are removed from the
	 * stack when returning to the launcher from an engine.
	 *
	 */
	void popAllCursors();

	/**
	 * Test whether cursor palettes are supported.
	 *
	 * This is just an convenience wrapper for checking for
	 * OSystem::kFeatureCursorPalette to be supported by OSystem.
	 *
	 * @see OSystem::kFeatureCursorPalette
	 * @see OSystem::hasFeature
	 */
	bool supportsCursorPalettes();

	/**
	 * Enable/Disable the current cursor palette.
	 *
	 * @param disable
	 */
	void disableCursorPalette(bool disable);

	/**
	 * Push a new cursor palette onto the stack, and set it in the backend.
	 * The palette entries from 'start' till (start+num-1) will be replaced
	 * so a full palette updated is accomplished via start=0, num=256.
	 *
	 * The palette data is specified in the same interleaved RGB format as
	 * used by all backends.
	 *
	 * @param colors	the new palette data, in interleaved RGB format
	 * @param start		the first palette entry to be updated
	 * @param num		the number of palette entries to be updated
	 *
	 * @note If num is zero, the cursor palette is disabled.
	 */
	void pushCursorPalette(const byte *colors, uint start, uint num);

	/**
	 * Pop a cursor palette from the stack, and restore the previous one to
	 * the backend. If there is no previous palette, the cursor palette is
	 * disabled instead.
	 */
	void popCursorPalette();

	/**
	 * Replace the current cursor palette on the stack. If the stack is
	 * empty, the palette is pushed instead. It's a slightly more optimized
	 * way of popping the old palette before pushing the new one.
	 *
	 * @param colors	the new palette data, in interleaved RGB format
	 * @param start		the first palette entry to be updated
	 * @param num		the number of palette entries to be updated
	 *
	 * @note If num is zero, the cursor palette is disabled.
	 */
	void replaceCursorPalette(const byte *colors, uint start, uint num);

private:
	friend class Common::Singleton<SingletonBaseType>;
	// Even though this is basically the default constructor we implement it
	// ourselves, so it is private and thus there is no way to create this class
	// except from the Singleton code.
	CursorManager() {}
	~CursorManager();

	struct Cursor {
		byte *_data;
		bool _visible;
		uint _width;
		uint _height;
		int _hotspotX;
		int _hotspotY;
		uint32 _keycolor;
		Graphics::PixelFormat _format;
		bool _dontScale;

		uint _size;

		Cursor(const void *data, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL);
		~Cursor();
	};

	struct Palette {
		byte *_data;
		uint _start;
		uint _num;
		uint _size;

		bool _disabled;

		Palette(const byte *colors, uint start, uint num);
		~Palette();
	};
	Common::Stack<Cursor *> _cursorStack;
	Common::Stack<Palette *> _cursorPaletteStack;
};

} // End of namespace Graphics

#define CursorMan	(::Graphics::CursorManager::instance())

#endif
