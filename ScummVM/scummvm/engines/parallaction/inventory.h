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

#ifndef PARALLACTION_INVENTORY_H
#define PARALLACTION_INVENTORY_H


#include "graphics/surface.h"

namespace Parallaction {

class Parallaction;

struct InventoryItem {
	uint32		_id;			// object name (lowest 16 bits are always zero)
	uint16		_index;			// index to frame in objs file
};

struct InventoryProperties {
	uint _itemPitch;
	uint _itemWidth;
	uint _itemHeight;

	int _maxItems;

	int _itemsPerLine;
	int _maxLines;

	int _width;
	int _height;
};

#define MAKE_INVENTORY_ID(x) (((x) & 0xFFFF) << 16)

typedef int16 ItemPosition;
typedef uint16 ItemName;

class Inventory {

protected:
	uint16			_numVerbs;

	InventoryItem	*_items;
	uint16			_numItems;
	int				_maxItems;

public:
	Inventory(int maxItems, InventoryItem *verbs);
	virtual ~Inventory();

	ItemPosition addItem(ItemName name, uint32 value);
	ItemPosition addItem(ItemName item);
	void removeItem(ItemName name);
	void clear(bool keepVerbs = true);

	const InventoryItem* getItem(ItemPosition pos) const;
	ItemName getItemName(ItemPosition pos) const;

	ItemPosition findItem(ItemName name) const;

	int16	getNumItems() const { return _numItems; }
};



class InventoryRenderer {
	Parallaction	*_vm;
	InventoryProperties *_props;

	Inventory		*_inv;
	Common::Point	_pos;

	Graphics::Surface	_surf;

protected:
	void getItemRect(ItemPosition pos, Common::Rect &r);

	void drawItem(ItemPosition pos, ItemName name);
	void refresh();

public:
	InventoryRenderer(Parallaction *vm, InventoryProperties *props, Inventory *inv);
	virtual ~InventoryRenderer();

	void showInventory();
	void hideInventory();

	ItemPosition hitTest(const Common::Point &p) const;
	void highlightItem(ItemPosition pos, byte color);
	void drawItem(ItemName name, byte *buffer, uint pitch);

	byte*	getData() const { return (byte *)_surf.pixels; }

	void	getRect(Common::Rect &r) const;
	int16	getNumLines() const;
};

} // namespace Parallaction

#endif
