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

#ifndef SCI_GRAPHICS_CONTROLS16_H
#define SCI_GRAPHICS_CONTROLS16_H

namespace Sci {

enum controlStateFlags {
	SCI_CONTROLS_STYLE_ENABLED  = 0x0001,  ///< 0001 - enabled buttons
	SCI_CONTROLS_STYLE_DISABLED = 0x0004,  ///< 0010 - grayed out buttons
	SCI_CONTROLS_STYLE_SELECTED = 0x0008   ///< 1000 - widgets surrounded by a frame
};

// Control types and flags
enum {
	SCI_CONTROLS_TYPE_BUTTON		= 1,
	SCI_CONTROLS_TYPE_TEXT			= 2,
	SCI_CONTROLS_TYPE_TEXTEDIT		= 3,
	SCI_CONTROLS_TYPE_ICON			= 4,
	SCI_CONTROLS_TYPE_LIST			= 6,
	SCI_CONTROLS_TYPE_LIST_ALIAS	= 7,
	SCI_CONTROLS_TYPE_DUMMY			= 10
};

class GfxPorts;
class GfxPaint16;
class Font;
class GfxText16;
class GfxScreen;
/**
 * Controls class, handles drawing of controls in SCI16 (SCI0-SCI1.1) games
 */
class GfxControls16 {
public:
	GfxControls16(SegManager *segMan, GfxPorts *ports, GfxPaint16 *paint16, GfxText16 *text16, GfxScreen *screen);
	~GfxControls16();

	void kernelDrawButton(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 style, bool hilite);
	void kernelDrawText(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 alignment, int16 style, bool hilite);
	void kernelDrawTextEdit(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 mode, int16 style, int16 cursorPos, int16 maxChars, bool hilite);
	void kernelDrawIcon(Common::Rect rect, reg_t obj, GuiResourceId viewId, int16 loopNo, int16 celNo, int16 priority, int16 style, bool hilite);
	void kernelDrawList(Common::Rect rect, reg_t obj, int16 maxChars, int16 count, const char **entries, GuiResourceId fontId, int16 style, int16 upperPos, int16 cursorPos, bool isAlias, bool hilite);
	void kernelTexteditChange(reg_t controlObject, reg_t eventObject);

private:
	void init();
	void texteditSetBlinkTime();

	void drawListControl(Common::Rect rect, reg_t obj, int16 maxChars, int16 count, const char **entries, GuiResourceId fontId, int16 upperPos, int16 cursorPos, bool isAlias);
	void texteditCursorDraw(Common::Rect rect, const char *text, uint16 curPos);
	void texteditCursorErase();
	int getPicNotValid();

	SegManager *_segMan;
	GfxPorts *_ports;
	GfxPaint16 *_paint16;
	GfxText16 *_text16;
	GfxScreen *_screen;

	// Textedit-Control related
	Common::Rect _texteditCursorRect;
	bool _texteditCursorVisible;
	uint32 _texteditBlinkTime;
};

} // End of namespace Sci

#endif
