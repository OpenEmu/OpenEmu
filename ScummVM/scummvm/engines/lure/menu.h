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

#ifndef LURE_MENU_H
#define LURE_MENU_H


#include "common/str.h"
#include "lure/luredefs.h"
#include "lure/disk.h"
#include "lure/screen.h"
#include "lure/surface.h"
#include "lure/events.h"

#define NUM_MENUS 3

namespace Lure {

struct MenuRecordBounds {
	uint16 left, right;
	uint16 contentsX, contentsWidth;
};

struct MenuRecordLanguage {
	Common::Language language;
	MenuRecordBounds menus[3];
};

class MenuRecord {
private:
	uint16 _xstart, _width;
	uint16 _hsxstart, _hsxend;
	const char **_entries;
	uint8 _numEntries;
public:
	MenuRecord(const MenuRecordBounds *bounds, int numParams, ...);
	~MenuRecord();

	uint16 xstart() { return _xstart; }
	uint16 width() { return _width; }
	uint16 hsxstart() { return _hsxstart; }
	uint16 hsxend() { return _hsxend; }
	uint8 numEntries() { return _numEntries; }
	const char **entries() { return _entries; }
	const char *getEntry(uint8 index);
};

class Menu {
private:
	MemoryBlock *_menu;
	MenuRecord *_menus[NUM_MENUS];
	MenuRecord *_selectedMenu;
	Surface *_surfaceMenu;
	uint8 _selectedIndex;

	MenuRecord *getMenuAt(int x);
	uint8 getIndexAt(uint16 x, uint16 y);
	void toggleHighlight(MenuRecord *menuRec);
	void toggleHighlightItem(uint8 index);
public:
	Menu();
	~Menu();
	static Menu &getReference();
	uint8 execute();
	MenuRecord &getMenu(uint8 index) { return *_menus[index]; }
};

class PopupMenu {
public:
	static Action Show(uint32 actionMask);
	static Action Show(int numEntries, Action *actions);
	static uint16 Show(int numEntries, const char *actions[]);
	static uint16 ShowInventory();
	static uint16 ShowItems(Action contextAction, uint16 roomNumber);
};

} // End of namespace Lure

#endif
