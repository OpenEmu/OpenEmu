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

#ifndef LASTEXPRESS_INVENTORY_H
#define LASTEXPRESS_INVENTORY_H

/*
	Inventory entry (32 entries)
	----------------------------

	    byte {1}        - Item ID (set to 0 for "undefined" items)
	    byte {1}        - Scene ID
	    byte {1}        - ??
	    byte {1}        - Selectable (1 if item is selectable, 0 otherwise)
	    byte {1}        - Is item in inventory (set to 1 for telegram and article)
	    byte {1}        - Auto selection (1 for no auto selection, 0 otherwise)
	    byte {1}        - Location

*/

#include "lastexpress/shared.h"

#include "lastexpress/eventhandler.h"

#include "common/events.h"
#include "common/serializer.h"

namespace LastExpress {

class LastExpressEngine;
class Scene;

class Inventory : Common::Serializable, public EventHandler {
public:

	// Entry
	struct InventoryEntry : Common::Serializable {
		CursorStyle cursor;
		SceneIndex scene;
		byte field_2;
		bool isSelectable;
		bool isPresent;
		bool manualSelect;
		ObjectLocation location;

		InventoryEntry() {
			cursor = kCursorNormal;
			scene = kSceneNone;
			field_2 = 0;
			isSelectable = false;
			isPresent = false;
			manualSelect = true;
			location = kObjectLocationNone;
		}

		Common::String toString() {
			return Common::String::format("{ %d - %d - %d - %d - %d - %d - %d }", cursor, scene, field_2, isSelectable, isPresent, manualSelect, location);
		}

		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncAsByte(cursor);
			s.syncAsByte(scene);
			s.syncAsByte(field_2);
			s.syncAsByte(isSelectable);
			s.syncAsByte(isPresent);
			s.syncAsByte(manualSelect);
			s.syncAsByte(location);
		}
	};

	Inventory(LastExpressEngine *engine);
	~Inventory();

	// Inventory contents
	void addItem(InventoryItem item);
	void removeItem(InventoryItem item, ObjectLocation newLocation = kObjectLocationNone);
	bool hasItem(InventoryItem item);
	void selectItem(InventoryItem item);
	void unselectItem();
	InventoryItem getSelectedItem() { return _selectedItem; }

	InventoryEntry *get(InventoryItem item);
	InventoryEntry *getSelectedEntry() { return get(_selectedItem); }

	InventoryItem getFirstExaminableItem() const;
	void setLocationAndProcess(InventoryItem item, ObjectLocation location);

	// UI Control
	void show();
	void showHourGlass() const;
	void setPortrait(InventoryItem item) const;
	void drawEgg() const;
	void drawBlinkingEgg(uint ticks = 1);

	// Handle inventory UI events.
	void handleMouseEvent(const Common::Event &ev);

	// State
	bool isMagnifierInUse() { return _useMagnifier; }
	bool isPortraitHighlighted() { return _portraitHighlighted; }
	bool isOpened() { return _isOpened; }
	bool isEggHighlighted() { return _eggHightlighted; }

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &s);
	void saveSelectedItem(Common::Serializer &s);

	/**
	 * Convert this object into a string representation.
	 *
	 * @return A string representation of this object.
	 */
	Common::String toString();

private:
	LastExpressEngine *_engine;

	InventoryEntry _entries[32];
	InventoryItem _selectedItem;
	uint32 _highlightedItemIndex;

	uint32 _itemsShown;

	bool _showingHourGlass;
	int16  _blinkingDirection;
	uint16 _blinkingBrightness;

	// Flags
	bool _useMagnifier;
	bool _portraitHighlighted;
	bool _isOpened;
	bool _eggHightlighted;

	Scene *_itemScene;

	Common::Rect _menuEggRect;
	Common::Rect _selectedItemRect;

	void init();

	void open();
	void close();
	void examine(InventoryItem item);
	void drawHighlight(uint32 currentIndex, bool reset);
	uint32 getItemIndex(uint32 currentIndex) const;

	bool isItemSceneParameter(InventoryItem item) const;

	void drawItem(CursorStyle id, uint16 x, uint16 y, int16 brighnessIndex = -1) const;
	void blinkEgg();

	void drawSelectedItem();
	void clearSelectedItem() const;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_INVENTORY_H
