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

#include "lure/menu.h"
#include "lure/luredefs.h"
#include "lure/decode.h"
#include "lure/surface.h"
#include "lure/res_struct.h"
#include "lure/res.h"
#include "lure/strings.h"
#include "lure/room.h"
#include "lure/events.h"
#include "lure/lure.h"

#if defined(_WIN32_WCE) || defined(__SYMBIAN32__) || defined(WEBOS)
#define LURE_CLICKABLE_MENUS
#endif

namespace Lure {

MenuRecord::MenuRecord(const MenuRecordBounds *bounds, int numParams, ...) {
	// Store list of pointers to strings
	va_list params;

	_numEntries = numParams;
	_entries = (const char **) malloc(sizeof(const char *) * _numEntries);

	va_start(params, numParams);
	for (int index = 0; index < _numEntries; ++index)
		_entries[index] = va_arg(params, const char *);

	// Store position data
	_hsxstart = bounds->left; _hsxend = bounds->right;
	_xstart = bounds->contentsX << 3;
	_width = (bounds->contentsWidth + 3) << 3;
}

MenuRecord::~MenuRecord() {
	free(_entries);
	_entries = NULL;
}

const char *MenuRecord::getEntry(uint8 index) {
	if (index >= _numEntries) error("Invalid menuitem index specified: %d", index);
	return _entries[index];
}

/*--------------------------------------------------------------------------*/

static Menu *int_menu = NULL;

const MenuRecordLanguage menuList[] = {
	{Common::EN_ANY, {{40, 87, 3, 7}, {127, 179, 13, 12}, {224, 281, 27, 10}}},
	{Common::IT_ITA, {{40, 98, 4, 6}, {120, 195, 14, 11}, {208, 281, 24, 13}}},
	{Common::FR_FRA, {{40, 90, 3, 7}, {120, 195, 13, 11}, {232, 273, 23, 13}}},
	{Common::DE_DEU, {{44, 95, 1, 11}, {135, 178, 8, 23}, {232, 273, 22, 15}}},
	{Common::ES_ESP, {{40, 90, 3, 8}, {120, 195, 11, 13}, {208, 281, 17, 18}}},
	{Common::UNK_LANG, {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}}
};

Menu::Menu() {
	int_menu = this;
	StringList &sl = Resources::getReference().stringList();
	Common::Language language = LureEngine::getReference().getLanguage();

	MemoryBlock *data = Disk::getReference().getEntry(MENU_RESOURCE_ID);
	PictureDecoder decoder;
	_menu = decoder.decode(data, SCREEN_SIZE);
	delete data;

	const MenuRecordLanguage *rec = &menuList[0];
	while ((rec->language != Common::UNK_LANG) && (rec->language != language))
		++rec;
	if (rec->language == Common::UNK_LANG)
		error("Unknown language encountered in top line handler");

	_menus[0] = new MenuRecord(&rec->menus[0], 1, sl.getString(S_CREDITS));
	_menus[1] = new MenuRecord(&rec->menus[1], 3,
		sl.getString(S_RESTART_GAME), sl.getString(S_SAVE_GAME), sl.getString(S_RESTORE_GAME));
	_menus[2] = new MenuRecord(&rec->menus[2], 3,
		sl.getString(S_QUIT), sl.getString(S_SLOW_TEXT), sl.getString(S_SOUND_ON));

	_selectedMenu = NULL;
}

Menu::~Menu() {
	for (int ctr=0; ctr<NUM_MENUS; ++ctr) delete _menus[ctr];
	delete _menu;
}

Menu &Menu::getReference() {
	return *int_menu;
}

uint8 Menu::execute() {
	OSystem &system = *g_system;
	LureEngine &engine = LureEngine::getReference();
	Mouse &mouse = Mouse::getReference();
	Events &events = Events::getReference();
	Screen &screen = Screen::getReference();

	mouse.setCursorNum(CURSOR_ARROW);
	system.copyRectToScreen(_menu->data(), FULL_SCREEN_WIDTH, 0, 0,
		FULL_SCREEN_WIDTH, MENUBAR_Y_SIZE);

	_selectedMenu = NULL;
	_surfaceMenu = NULL;
	_selectedIndex = 0;

	while (mouse.lButton() || mouse.rButton()) {
		while (events.pollEvent()) {
			if (engine.shouldQuit()) return MENUITEM_NONE;

			if (mouse.y() < MENUBAR_Y_SIZE) {
				MenuRecord *p = getMenuAt(mouse.x());

				if (_selectedMenu != p) {
					// If necessary, remove prior menu
					if (_selectedMenu) {
						toggleHighlight(_selectedMenu);
						screen.updateArea(0, 0, FULL_SCREEN_WIDTH, _surfaceMenu->height() + 8);
						delete _surfaceMenu;
						_surfaceMenu = NULL;
						_selectedIndex = 0;
					}

					_selectedMenu = p;

					// If a new menu is selected, show it
					if (_selectedMenu) {
						toggleHighlight(_selectedMenu);
						_surfaceMenu = Surface::newDialog(
							_selectedMenu->width(), _selectedMenu->numEntries(),
							_selectedMenu->entries(), false, DEFAULT_TEXT_COLOR, false);
						_surfaceMenu->copyToScreen(_selectedMenu->xstart(), MENUBAR_Y_SIZE);
					}

					system.copyRectToScreen(_menu->data(), FULL_SCREEN_WIDTH, 0, 0,
						FULL_SCREEN_WIDTH, MENUBAR_Y_SIZE);
				}
			}

			// Check for changing selected index
			uint8 index = getIndexAt(mouse.x(), mouse.y());
			if (index != _selectedIndex) {
				if (_selectedIndex != 0) toggleHighlightItem(_selectedIndex);
				_selectedIndex = index;
				if (_selectedIndex != 0) toggleHighlightItem(_selectedIndex);
			}
		}

		system.updateScreen();
		system.delayMillis(10);
	}

	delete _surfaceMenu;

	// Deselect the currently selected menu header
	if (_selectedMenu)
		toggleHighlight(_selectedMenu);

	// Restore the previous screen
	screen.update();

	if ((_selectedMenu == NULL) || (_selectedIndex == 0)) return MENUITEM_NONE;
	else if (_selectedMenu == _menus[0])
		return MENUITEM_CREDITS;
	else if (_selectedMenu == _menus[1]) {
		switch (_selectedIndex) {
		case 1:
			return MENUITEM_RESTART_GAME;
		case 2:
			return MENUITEM_SAVE_GAME;
		case 3:
			return MENUITEM_RESTORE_GAME;
		}
	} else {
		switch (_selectedIndex) {
		case 1:
			return MENUITEM_QUIT;
		case 2:
			return MENUITEM_TEXT_SPEED;
		case 3:
			return MENUITEM_SOUND;
		}
	}
	return MENUITEM_NONE;
}

MenuRecord *Menu::getMenuAt(int x) {
	for (int ctr = 0; ctr < NUM_MENUS; ++ctr)
		if ((x >= _menus[ctr]->hsxstart()) && (x <= _menus[ctr]->hsxend()))
			return _menus[ctr];

	return NULL;
}

uint8 Menu::getIndexAt(uint16 x, uint16 y) {
	if (!_selectedMenu) return 0;

	int ys = MENUBAR_Y_SIZE + Surface::textY();
	int ye = MENUBAR_Y_SIZE + (_surfaceMenu->height() - Surface::textY());
	if ((y < ys) || (y > ye)) return 0;

	uint16 yRelative = y - ys;
	uint8 index = (uint8) (yRelative / 8) + 1;
	if (index > _selectedMenu->numEntries()) index = _selectedMenu->numEntries();
	return index;
}

#define MENUBAR_SELECTED_COLOR 0xf7

void Menu::toggleHighlight(MenuRecord *menuRec) {
	const byte colorList[4] = {4, 2, 0, 0xf7};
	const byte *colors = LureEngine::getReference().isEGA() ? &colorList[0] : &colorList[2];
	byte *addr = _menu->data();

	for (uint16 y=0; y<MENUBAR_Y_SIZE; ++y) {
		for (uint16 x=menuRec->hsxstart(); x<=menuRec->hsxend(); ++x) {
			if (addr[x] == colors[0]) addr[x] = colors[1];
			else if (addr[x] == colors[1]) addr[x] = colors[0];
		}
		addr += FULL_SCREEN_WIDTH;
	}
}

void Menu::toggleHighlightItem(uint8 index) {
	const byte colorList[4] = {EGA_DIALOG_TEXT_COLOR, EGA_DIALOG_WHITE_COLOR,
		VGA_DIALOG_TEXT_COLOR, VGA_DIALOG_WHITE_COLOR};
	const byte *colors = LureEngine::getReference().isEGA() ? &colorList[0] : &colorList[2];
	byte *p = _surfaceMenu->data().data() + (Surface::textY() +
		((index - 1) * FONT_HEIGHT)) * _surfaceMenu->width() + Surface::textX();
	int numBytes =_surfaceMenu->width() - Surface::textX() * 2;

	for (int y = 0; y < FONT_HEIGHT; ++y, p += _surfaceMenu->width()) {
		byte *pTemp = p;

		for (int x = 0; x < numBytes; ++x, ++pTemp) {
			if (*pTemp == colors[0]) *pTemp = colors[1];
			else if (*pTemp == colors[1]) *pTemp = colors[0];
		}
	}

	_surfaceMenu->copyToScreen(_selectedMenu->xstart(), MENUBAR_Y_SIZE);
}

/*--------------------------------------------------------------------------*/

uint16 PopupMenu::ShowInventory() {
	Resources &rsc = Resources::getReference();
	StringData &strings = StringData::getReference();

	uint16 numItems = rsc.numInventoryItems();
	uint16 itemCtr = 0;
	char **itemNames = (char **) Memory::alloc(sizeof(char *) * numItems);
	uint16 *idList = (uint16 *) Memory::alloc(sizeof(uint16) * numItems);

	HotspotDataList::iterator i;
	for (i = rsc.hotspotData().begin(); i != rsc.hotspotData().end(); ++i) {
		HotspotData const &hotspot = **i;
		if (hotspot.roomNumber == PLAYER_ID) {
			idList[itemCtr] = hotspot.hotspotId;
			char *hotspotName = itemNames[itemCtr++] = (char *) malloc(MAX_HOTSPOT_NAME_SIZE);
			strings.getString(hotspot.nameId, hotspotName);
		}
	}

	uint16 result = Show(numItems, const_cast<const char **>(itemNames));
	if (result != 0xffff) result = idList[result];

	for (itemCtr = 0; itemCtr < numItems; ++itemCtr)
		free(itemNames[itemCtr]);

	Memory::dealloc(itemNames);
	Memory::dealloc(idList);
	return result;
}

#define MAX_NUM_DISPLAY_ITEMS 20

uint16 PopupMenu::ShowItems(Action contextAction, uint16 roomNumber) {
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	RoomDataList &rooms = res.roomData();
	HotspotDataList &hotspots = res.hotspotData();
	StringData &strings = StringData::getReference();
	Room &room = Room::getReference();
	Screen &screen = Screen::getReference();
	Mouse &mouse = Mouse::getReference();
	RoomDataList::iterator ir;
	HotspotDataList::iterator ih;
	uint16 entryIds[MAX_NUM_DISPLAY_ITEMS];
	uint16 nameIds[MAX_NUM_DISPLAY_ITEMS];
	char *entryNames[MAX_NUM_DISPLAY_ITEMS];
	int numItems = 0;
	int itemCtr;
	uint32 contextBitflag = 1 << (contextAction - 1);

	// Loop for rooms
	for (ir = rooms.begin(); ir != rooms.end(); ++ir) {
		RoomData const &roomData = **ir;
		// Pre-condition checks for whether to skip room
		if ((roomData.hdrFlags != 15) && ((roomData.hdrFlags & fields.hdrFlagMask()) == 0))
			continue;
		if (((roomData.flags & HOTSPOTFLAG_MENU_EXCLUSION) != 0) || ((roomData.flags & HOTSPOTFLAG_FOUND) == 0))
			continue;
		if ((roomData.actions & contextBitflag) == 0)
			continue;

		// Add room to list of entries to display
		if (numItems == MAX_NUM_DISPLAY_ITEMS) error("Out of space in ask list");
		entryIds[numItems] = roomData.roomNumber;
		nameIds[numItems] = roomData.roomNumber;
		entryNames[numItems] = (char *) Memory::alloc(MAX_HOTSPOT_NAME_SIZE);
		strings.getString(roomData.roomNumber, entryNames[numItems]);
		++numItems;
	}

	// Loop for hotspots
	for (ih = hotspots.begin(); ih != hotspots.end(); ++ih) {
		HotspotData const &hotspot = **ih;

		if ((hotspot.headerFlags != 15) &&
			((hotspot.headerFlags & fields.hdrFlagMask()) == 0))
			continue;

		if (((hotspot.flags & HOTSPOTFLAG_MENU_EXCLUSION) != 0) || ((hotspot.flags & HOTSPOTFLAG_FOUND) == 0))
			// Skip the current hotspot
			continue;

		// If the hotspot is room specific, skip if the character will not be in the specified room
		if (((hotspot.flags & HOTSPOTFLAG_ROOM_SPECIFIC) != 0) &&
			(hotspot.roomNumber != roomNumber))
			continue;

		// If hotspot does not allow action, then skip it
		if ((hotspot.actions & contextBitflag) == 0)
			continue;

		// If a special hotspot Id, then skip displaying
		if ((hotspot.nameId == 0x17A) || (hotspot.nameId == 0x147))
			continue;

		// Check if the hotspot's name is already used in an already set item
		itemCtr = 0;
		while ((itemCtr < numItems) && (nameIds[itemCtr] != hotspot.nameId))
			++itemCtr;
		if (itemCtr != numItems)
			// Item's name is already present - skip hotspot
			continue;

		// Add hotspot to list of entries to display
		if (numItems == MAX_NUM_DISPLAY_ITEMS) error("Out of space in ask list");
		entryIds[numItems] = hotspot.hotspotId;
		nameIds[numItems] = hotspot.nameId;
		entryNames[numItems] = (char *) Memory::alloc(MAX_HOTSPOT_NAME_SIZE);
		strings.getString(hotspot.nameId, entryNames[numItems]);
		++numItems;
	}

	if (numItems == 0)
		// No items, so add a 'nothing' to the statusLine
		strcat(room.statusLine(), "(nothing)");

	room.update();
	screen.update();
	mouse.waitForRelease();

	if (numItems == 0)
		// Return flag for no items to ask for
		return 0xfffe;

	// Display items
	uint16 result = Show(numItems, const_cast<const char **>(entryNames));
	if (result != 0xffff) result = entryIds[result];

	// Deallocate display strings
	for (itemCtr = 0; itemCtr < numItems; ++itemCtr)
		Memory::dealloc(entryNames[itemCtr]);

	return result;
}

static int entryCompare(const char **p1, const char **p2) {
	return strcmp(*p1, *p2);
}

typedef int (*CompareMethod)(const void*, const void*);

Action PopupMenu::Show(uint32 actionMask) {
	StringList &stringList = Resources::getReference().stringList();
	int numEntries = 0;
	uint32 v = actionMask;
	int index;
	int currentAction;
	uint16 resultIndex;
	Action resultAction;

	for (index = 1; index <= EXAMINE; ++index, v >>= 1) {
		if (v & 1) ++numEntries;
	}

	const char **strList = (const char **) Memory::alloc(sizeof(char *) * numEntries);

	int strIndex = 0;
	for (currentAction = 0; currentAction < (int)EXAMINE; ++currentAction) {
		if ((actionMask & (1 << currentAction)) != 0) {
			strList[strIndex] = stringList.getString(currentAction);
			++strIndex;
		}
	}

	// Sort the list
	qsort(strList, numEntries, sizeof(const char *), (CompareMethod) entryCompare);

	// Show the entries
	resultIndex = Show(numEntries, strList);

	resultAction = NONE;
	if (resultIndex != 0xffff) {
		// Scan through the list of actions to find the selected entry
		for (currentAction = 0; currentAction < (int)EXAMINE; ++currentAction) {
			if (strList[resultIndex] == stringList.getString(currentAction)) {
				resultAction = (Action) (currentAction + 1);
				break;
			}
		}
	}

	Memory::dealloc(strList);
	return resultAction;
}

Action PopupMenu::Show(int numEntries, Action *actions) {
	StringList &stringList = Resources::getReference().stringList();
	const char **strList = (const char **) Memory::alloc(sizeof(char *) * numEntries);
	Action *actionPtr = actions;
	for (int index = 0; index < numEntries; ++index)
		strList[index] = stringList.getString(*actionPtr++);
	uint16 result = Show(numEntries, strList);

	delete strList;
	if (result == 0xffff) return NONE;
	else return actions[result];
}

uint16 PopupMenu::Show(int numEntries, const char *actions[]) {
	if (numEntries == 0) return 0xffff;
	LureEngine &engine = LureEngine::getReference();
	Events &e = Events::getReference();
	Mouse &mouse = Mouse::getReference();
	OSystem &system = *g_system;
	Screen &screen = Screen::getReference();
	Common::Rect r;
	bool isEGA = LureEngine::getReference().isEGA();
	byte bgColor = isEGA ? EGA_DIALOG_BG_COLOR : 0;
	byte textColor = isEGA ? EGA_DIALOG_TEXT_COLOR : VGA_DIALOG_TEXT_COLOR;
	byte whiteColor = isEGA ? EGA_DIALOG_WHITE_COLOR : VGA_DIALOG_WHITE_COLOR;


	const uint16 yMiddle = FULL_SCREEN_HEIGHT / 2;
#ifndef LURE_CLICKABLE_MENUS
	uint16 oldX = mouse.x();
	uint16 oldY = mouse.y();
	mouse.cursorOff();
	mouse.setPosition(FULL_SCREEN_WIDTH / 2, yMiddle);

	// Round up number of lines in dialog to next odd number
	uint16 numLines = (numEntries / 2) * 2 + 1;
	if (numLines > 5) numLines = 5;
#else
	mouse.pushCursorNum(CURSOR_ARROW);

	// In WinCE, the whole menu is shown and the items are click-selectable
	uint16 numLines = numEntries;
#endif

	// Figure out the character width
	uint16 numCols = 0;
	for (int ctr = 0; ctr < numEntries; ++ctr) {
		int len = strlen(actions[ctr]);
		if (len > numCols)
			numCols = len;
	}

	// Create the dialog surface
	Common::Point size;
	Surface::getDialogBounds(size, numCols, numLines, false);
	Surface *s = new Surface(size.x, size.y);
	s->createDialog(true);

	int selectedIndex = 0;
	bool refreshFlag = true;
	r.left = Surface::textX();
	r.right = s->width() - Surface::textX() + 1;
	r.top = Surface::textY();
	r.bottom = s->height() - Surface::textY() + 1;

	bool bailOut = false;

	while (!bailOut) {
		if (refreshFlag) {
			// Set up the contents of the menu
			s->fillRect(r, bgColor);

			for (int index = 0; index < numLines; ++index) {
#ifndef LURE_CLICKABLE_MENUS
				int actionIndex = selectedIndex - (numLines / 2) + index;
#else
				int actionIndex = index;
#endif
				if ((actionIndex >= 0) && (actionIndex < numEntries)) {
					s->writeString(Surface::textX(), Surface::textY() + index * FONT_HEIGHT,
						actions[actionIndex], true,
#ifndef LURE_CLICKABLE_MENUS
						(index == (numLines / 2)) ? whiteColor : textColor,
#else
						(index == selectedIndex) ? whiteColor : textColor,
#endif
						false);
				}
			}

			s->copyToScreen(0, yMiddle-(s->height() / 2));
			system.updateScreen();
			refreshFlag = false;
		}

		while (e.pollEvent()) {
			if (engine.shouldQuit()) {
				selectedIndex = 0xffff;
				bailOut = true;
				break;
			} else if (e.type() == Common::EVENT_WHEELUP) {
				// Scroll upwards
				if (selectedIndex > 0) {
					--selectedIndex;
					refreshFlag = true;
				}
			} else if (e.type() == Common::EVENT_WHEELDOWN) {
				// Scroll downwards
				if (selectedIndex < numEntries - 1) {
					++selectedIndex;
					refreshFlag = true;
				}
			} else if (e.type() == Common::EVENT_KEYDOWN) {
				uint16 keycode = e.event().kbd.keycode;

				if (((keycode == Common::KEYCODE_KP8) || (keycode == Common::KEYCODE_UP)) && (selectedIndex > 0)) {
					--selectedIndex;
					refreshFlag = true;
				} else if (((keycode == Common::KEYCODE_KP2) || (keycode == Common::KEYCODE_DOWN)) &&
						(selectedIndex < numEntries-1)) {
					++selectedIndex;
					refreshFlag = true;
				} else if ((keycode == Common::KEYCODE_RETURN) || (keycode == Common::KEYCODE_KP_ENTER)) {
					bailOut = true;
					break;
				} else if (keycode == Common::KEYCODE_ESCAPE) {
					selectedIndex = 0xffff;
					bailOut = true;
					break;
				}

#ifdef LURE_CLICKABLE_MENUS
			} else if (e.type() == Common::EVENT_LBUTTONDOWN || e.type() == Common::EVENT_MOUSEMOVE) {
				int16 x = mouse.x();
				int16 y = mouse.y() - yMiddle + (s->height() / 2);
				refreshFlag = true;

				if (r.contains(x, y)) {
					selectedIndex = (y - r.top) / FONT_HEIGHT;
					if (e.type() == Common::EVENT_LBUTTONDOWN)
						bailOut = true;
						break;
				}
#else
			} else if ((e.type() == Common::EVENT_LBUTTONDOWN) ||
					(e.type() == Common::EVENT_MBUTTONDOWN)) {
				//mouse.waitForRelease();
				bailOut = true;
				break;
#endif
			} else if (e.type() == Common::EVENT_RBUTTONDOWN) {
				mouse.waitForRelease();
				selectedIndex = 0xffff;
				bailOut = true;
				break;
			}
		}

		if (!bailOut) {
#ifndef LURE_CLICKABLE_MENUS
			// Warping the mouse to "neutral" even if the top/bottom menu
			// entry has been reached has both pros and cons. It makes the
			// menu behave a bit more sensibly, but it also makes it harder
			// to move the mouse pointer out of the ScummVM window.

			if (mouse.y() < yMiddle - POPMENU_CHANGE_SENSITIVITY) {
				if (selectedIndex > 0) {
					--selectedIndex;
					refreshFlag = true;
				}
				mouse.setPosition(FULL_SCREEN_WIDTH / 2, yMiddle);
			} else if (mouse.y() > yMiddle + POPMENU_CHANGE_SENSITIVITY) {
				if (selectedIndex < numEntries - 1) {
					++selectedIndex;
					refreshFlag = true;
				}
				mouse.setPosition(FULL_SCREEN_WIDTH / 2, yMiddle);
			}
#endif

			system.delayMillis(20);
		}
	}

	// bailOut
	delete s;

#ifndef LURE_CLICKABLE_MENUS
	mouse.setPosition(oldX, oldY);
	mouse.cursorOn();
#else
	mouse.popCursor();
#endif
	screen.update();
	return selectedIndex;
}

} // End of namespace Lure
