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

#include "parallaction/input.h"
#include "parallaction/parallaction.h"

#include "common/textconsole.h"

namespace Parallaction {


/*
#define INVENTORYITEM_PITCH			32
#define INVENTORYITEM_WIDTH			24
#define INVENTORYITEM_HEIGHT		24

#define INVENTORY_MAX_ITEMS			30

#define INVENTORY_ITEMS_PER_LINE	5
#define INVENTORY_LINES				6

#define INVENTORY_WIDTH				(INVENTORY_ITEMS_PER_LINE*INVENTORYITEM_WIDTH)
#define INVENTORY_HEIGHT			(INVENTORY_LINES*INVENTORYITEM_HEIGHT)
*/

InventoryItem _verbs_NS[] = {
	{ 1, kZoneDoor },
	{ 3, kZoneExamine },
	{ 2, kZoneGet },
	{ 4, kZoneSpeak },
	{ 0, 0 }
};

InventoryItem _verbs_BR[] = {
	{ 1, kZoneBox },
	{ 2, kZoneGet },
	{ 3, kZoneExamine },
	{ 4, kZoneSpeak },
	{ 0, 0 }
};

InventoryProperties	_invProps_NS = {
	32,		// INVENTORYITEM_PITCH
	24,		// INVENTORYITEM_WIDTH
	24,		// INVENTORYITEM_HEIGHT
	30,		// INVENTORY_MAX_ITEMS
	5,		// INVENTORY_ITEMS_PER_LINE
	6,		// INVENTORY_LINES
	5 * 24,		// INVENTORY_WIDTH =(INVENTORY_ITEMS_PER_LINE*INVENTORYITEM_WIDTH)
	6 * 24		// INVENTORY_HEIGHT = (INVENTORY_LINES*INVENTORYITEM_HEIGHT)
};

InventoryProperties	_invProps_BR = {
	51,		// INVENTORYITEM_PITCH
	51,		// INVENTORYITEM_WIDTH
	51,		// INVENTORYITEM_HEIGHT
	48,		// INVENTORY_MAX_ITEMS
	6,		// INVENTORY_ITEMS_PER_LINE
	8,		// INVENTORY_LINES
	6 * 51,		// INVENTORY_WIDTH =(INVENTORY_ITEMS_PER_LINE*INVENTORYITEM_WIDTH)
	8 * 51		// INVENTORY_HEIGHT = (INVENTORY_LINES*INVENTORYITEM_HEIGHT)
};

int16 Parallaction::getHoverInventoryItem(int16 x, int16 y) {
	return _inventoryRenderer->hitTest(Common::Point(x,y));
}

void Parallaction::highlightInventoryItem(ItemPosition pos) {
	static ItemPosition lastHighlightedPos = -1;

	if (lastHighlightedPos != -1) {
		_inventoryRenderer->highlightItem(lastHighlightedPos, 12);
	}

	if (pos != -1) {
		_inventoryRenderer->highlightItem(pos, 19);
	}

	lastHighlightedPos = pos;
}

int Parallaction::addInventoryItem(ItemName item) {
	return _inventory->addItem(item);
}

int Parallaction::addInventoryItem(ItemName item, uint32 value) {
	return _inventory->addItem(item, value);
}

void Parallaction::dropItem(uint16 v) {
	_inventory->removeItem(v);
}

bool Parallaction::isItemInInventory(int32 v) {
	return (_inventory->findItem(v) != -1);
}

const InventoryItem* Parallaction::getInventoryItem(int16 pos) {
	return _inventory->getItem(pos);
}

int16 Parallaction::getInventoryItemIndex(int16 pos) {
	return _inventory->getItemName(pos);
}

void Parallaction::cleanInventory(bool keepVerbs) {
	_inventory->clear(keepVerbs);
}

void Parallaction::openInventory() {
	_inventoryRenderer->showInventory();
}

void Parallaction::closeInventory() {
	_inventoryRenderer->hideInventory();
}




InventoryRenderer::InventoryRenderer(Parallaction *vm, InventoryProperties *props, Inventory *inv) : _vm(vm), _props(props), _inv(inv) {
	_surf.create(_props->_width, _props->_height, Graphics::PixelFormat::createFormatCLUT8());
}

InventoryRenderer::~InventoryRenderer() {
	_surf.free();
}

void InventoryRenderer::showInventory() {
	if (!_inv)
		error("InventoryRenderer not bound to inventory");

	uint16 lines = getNumLines();

	Common::Point p;
	_vm->_input->getCursorPos(p);

	_pos.x = CLIP((int)(p.x - (_props->_width / 2)), 0, (int)(_vm->_screenWidth - _props->_width));
	_pos.y = CLIP((int)(p.y - 2 - (lines * _props->_itemHeight)), 0, (int)(_vm->_screenHeight - lines * _props->_itemHeight));

	refresh();
}

void InventoryRenderer::hideInventory() {
	if (!_inv)
		error("InventoryRenderer not bound to inventory");
}

void InventoryRenderer::getRect(Common::Rect& r) const {
	r.setWidth(_props->_width);
	r.setHeight(_props->_itemHeight * getNumLines());
	r.moveTo(_pos);
}

ItemPosition InventoryRenderer::hitTest(const Common::Point &p) const {
	Common::Rect r;
	getRect(r);
	if (!r.contains(p))
		return -1;

	return ((p.x - _pos.x) / _props->_itemWidth) + (_props->_itemsPerLine * ((p.y - _pos.y) / _props->_itemHeight));
}

void InventoryRenderer::drawItem(ItemPosition pos, ItemName name) {
	Common::Rect r;
	getItemRect(pos, r);
	byte* d = (byte *)_surf.getBasePtr(r.left, r.top);
	drawItem(name, d, _surf.pitch);
}

void InventoryRenderer::drawItem(ItemName name, byte *buffer, uint pitch) {
	byte* s = _vm->_objects->getData(name);
	byte* d = buffer;
	for (uint i = 0; i < _props->_itemHeight; i++) {
		memcpy(d, s, _props->_itemWidth);

		s += _props->_itemPitch;
		d += pitch;
	}
}


int16 InventoryRenderer::getNumLines() const {
	int16 num = _inv->getNumItems();
	return (num / _props->_itemsPerLine) + ((num % _props->_itemsPerLine) > 0 ? 1 : 0);
}


void InventoryRenderer::refresh() {
	for (uint16 i = 0; i < _props->_maxItems; i++) {
		ItemName name = _inv->getItemName(i);
		drawItem(i, name);
	}
}

void InventoryRenderer::highlightItem(ItemPosition pos, byte color) {
	if (pos == -1)
		return;

	Common::Rect r;
	getItemRect(pos, r);

	if (color != 12)
		color = 19;

	_surf.frameRect(r, color);
}

void InventoryRenderer::getItemRect(ItemPosition pos, Common::Rect &r) {

	r.setHeight(_props->_itemHeight);
	r.setWidth(_props->_itemWidth);

	uint16 line = pos / _props->_itemsPerLine;
	uint16 col = pos % _props->_itemsPerLine;

	r.moveTo(col * _props->_itemWidth, line * _props->_itemHeight);

}

Inventory::Inventory(int maxItems, InventoryItem *verbs) : _numItems(0), _maxItems(maxItems) {
	_items = (InventoryItem *)calloc(_maxItems, sizeof(InventoryItem));

	int i = 0;
	for ( ; verbs[i]._id; i++) {
		addItem(verbs[i]._id, verbs[i]._index);
	}
	_numVerbs = i;
}


Inventory::~Inventory() {
	free(_items);
}

ItemPosition Inventory::addItem(ItemName name, uint32 value) {
	debugC(1, kDebugInventory, "addItem(%i, %i)", name, value);

	if (_numItems == _maxItems) {
		debugC(3, kDebugInventory, "addItem: inventory is full");
		return -1;
	}

	// NOTE: items whose name == 0 aren't really inventory items,
	// but the engine expects the inventory to accept them as valid.
	// This nasty trick has been discovered because of regression
	// after r29060.
	if (name == 0)
		return 0;

	_items[_numItems]._id = value;
	_items[_numItems]._index = name;

	_numItems++;

	debugC(3, kDebugInventory, "addItem: done");

	return _numItems;
}

ItemPosition Inventory::addItem(ItemName name) {
	return addItem(name, MAKE_INVENTORY_ID(name));
}

ItemPosition Inventory::findItem(ItemName name) const {
	for (ItemPosition slot = 0; slot < _numItems; slot++) {
		if (name == _items[slot]._index)
			return slot;
	}

	return -1;
}

void Inventory::removeItem(ItemName name) {
	debugC(1, kDebugInventory, "removeItem(%i)", name);

	ItemPosition pos = findItem(name);
	if (pos == -1) {
		debugC(3, kDebugInventory, "removeItem: can't find item, nothing to remove");
		return;
	}

	_numItems--;

	if (_numItems != pos) {
		memmove(&_items[pos], &_items[pos+1], (_numItems - pos) * sizeof(InventoryItem));
	}

	_items[_numItems]._id = 0;
	_items[_numItems]._index = 0;

	debugC(3, kDebugInventory, "removeItem: item removed");
}

void Inventory::clear(bool keepVerbs) {
	debugC(1, kDebugInventory, "clearInventory()");

	uint first = (keepVerbs ? _numVerbs : 0);

	for (uint16 slot = first; slot < _numItems; slot++) {
		_items[slot]._id = 0;
		_items[slot]._index = 0;
	}

	_numItems = first;
}


ItemName Inventory::getItemName(ItemPosition pos) const {
	return (pos >= 0 && pos < _maxItems) ? _items[pos]._index : 0;
}

const InventoryItem* Inventory::getItem(ItemPosition pos) const {
	return &_items[pos];
}



void Parallaction_ns::initInventory() {
	_inventory = new Inventory(_invProps_NS._maxItems, _verbs_NS);
	assert(_inventory);
	_inventoryRenderer = new InventoryRenderer(this, &_invProps_NS, _inventory);
	assert(_inventoryRenderer);
}

void Parallaction_br::initInventory() {
	_inventory = new Inventory(_invProps_BR._maxItems, _verbs_BR);
	assert(_inventory);
	_inventoryRenderer = new InventoryRenderer(this, &_invProps_BR, _inventory);
	assert(_inventoryRenderer);

	_charInventories[0] = new Inventory(_invProps_BR._maxItems, _verbs_BR);
	_charInventories[1] = new Inventory(_invProps_BR._maxItems, _verbs_BR);
	_charInventories[2] = new Inventory(_invProps_BR._maxItems, _verbs_BR);
}

void Parallaction_ns::destroyInventory() {
	delete _inventoryRenderer;
	delete _inventory;
	_inventory = 0;
	_inventoryRenderer = 0;
}

void Parallaction_br::destroyInventory() {
	delete _inventoryRenderer;
	delete _inventory;
	_inventory = 0;
	_inventoryRenderer = 0;

	delete _charInventories[0];
	delete _charInventories[1];
	delete _charInventories[2];
	_charInventories[0] = 0;
	_charInventories[1] = 0;
	_charInventories[2] = 0;
}


} // namespace Parallaction
