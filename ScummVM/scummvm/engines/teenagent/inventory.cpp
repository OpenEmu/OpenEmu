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

#include "common/memstream.h"
#include "common/ptr.h"
#include "common/textconsole.h"

#include "teenagent/inventory.h"

#include "teenagent/resources.h"
#include "teenagent/objects.h"
#include "teenagent/teenagent.h"
#include "teenagent/scene.h"

namespace TeenAgent {

Inventory::Inventory(TeenAgentEngine *vm) : _vm(vm) {
	_active = false;

	FilePack varia;
	varia.open("varia.res");

	Common::ScopedPtr<Common::SeekableReadStream> s(varia.getStream(3));
	if (!s)
		error("no inventory background");
	debugC(0, kDebugInventory, "loading inventory background...");
	_background.load(*s, Surface::kTypeOns);

	uint32 itemsSize = varia.getSize(4);
	if (itemsSize == 0)
		error("invalid inventory items size");
	debugC(0, kDebugInventory, "loading items, size: %u", itemsSize);
	_items = new byte[itemsSize];
	varia.read(4, _items, itemsSize);

	byte offsets = _items[0];
	assert(offsets == kNumInventoryItems);
	for (byte i = 0; i < offsets; ++i) {
		_offset[i] = READ_LE_UINT16(_items + i * 2 + 1);
	}
	_offset[kNumInventoryItems] = itemsSize;

	InventoryObject ioBlank;
	_objects.push_back(ioBlank);
	for (byte i = 0; i < kNumInventoryItems; ++i) {
		InventoryObject io;
		uint16 objAddr = vm->res->dseg.get_word(dsAddr_inventoryItemDataPtrTable + i * 2);
		io.load(vm->res->dseg.ptr(objAddr));
		_objects.push_back(io);
	}

	_inventory = vm->res->dseg.ptr(dsAddr_inventory);

	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 6; ++x) {
			int i = y * 6 + x;
			_graphics[i]._rect.left = 28 + 45 * x - 1;
			_graphics[i]._rect.top = 23 + 31 * y - 1;
			_graphics[i]._rect.right = _graphics[i]._rect.left + 40;
			_graphics[i]._rect.bottom = _graphics[i]._rect.top + 26;
		}
	}

	varia.close();
	_hoveredObj = _selectedObj = NULL;
}

Inventory::~Inventory() {
	delete[] _items;
}

bool Inventory::has(byte item) const {
	for (int i = 0; i < kInventorySize; ++i) {
		if (_inventory[i] == item)
			return true;
	}
	return false;
}

void Inventory::remove(byte item) {
	debugC(0, kDebugInventory, "removing %u from inventory", item);
	int i;
	for (i = 0; i < kInventorySize; ++i) {
		if (_inventory[i] == item) {
			break;
		}
	}
	for (; i < (kInventorySize - 1); ++i) {
		_inventory[i] = _inventory[i + 1];
		_graphics[i].free();
	}
	_inventory[kInventorySize - 1] = kInvItemNoItem;
	_graphics[kInventorySize - 1].free();
}

void Inventory::clear() {
	debugC(0, kDebugInventory, "clearing inventory");
	for (int i = 0; i < kInventorySize; ++i) {
		_inventory[i] = kInvItemNoItem;
		_graphics[i].free();
	}
}

void Inventory::reload() {
	for (int i = 0; i < kInventorySize; ++i) {
		_graphics[i].free();
		uint item = _inventory[i];
		if (item != kInvItemNoItem)
			_graphics[i].load(this, item);
	}
}

void Inventory::add(byte item) {
	if (has(item))
		return;
	debugC(0, kDebugInventory, "adding %u to inventory", item);
	for (int i = 0; i < kInventorySize; ++i) {
		if (_inventory[i] == kInvItemNoItem) {
			_inventory[i] = item;
			return;
		}
	}
	error("no room for item %u", item);
}

bool Inventory::tryObjectCallback(InventoryObject *obj) {
	byte objId = obj->id;
	for (uint i = 0; i < 7; ++i) {
		byte tableId = _vm->res->dseg.get_byte(dsAddr_objCallbackTablePtr + (3 * i));
		uint16 callbackAddr = _vm->res->dseg.get_word(dsAddr_objCallbackTablePtr + (3 * i) + 1);
		if (tableId == objId) {
			resetSelectedObject();
			activate(false);
			if (_vm->processCallback(callbackAddr))
				return true;
		}
	}
	return false;
}

bool Inventory::processEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:

		if (!_active) {
			if (event.mouse.y < 5)
				activate(true);
			_mouse = event.mouse;
			return false;
		}

		if (event.mouse.x < 17 || event.mouse.x >= 303 || (event.mouse.y - _mouse.y > 0 && event.mouse.y >= 153)) {
			activate(false);
			_mouse = event.mouse;
			return false;
		}

		_mouse = event.mouse;
		_hoveredObj = NULL;

		for (int i = 0; i < kInventorySize; ++i) {
			byte item = _inventory[i];
			if (item == kInvItemNoItem)
				continue;

			_graphics[i]._hovered = _graphics[i]._rect.in(_mouse);
			if (_graphics[i]._hovered)
				_hoveredObj = &_objects[item];
		}
		return true;

	case Common::EVENT_LBUTTONDOWN: {
		//check combine
		if (!_active)
			return false;

		if (_hoveredObj == NULL)
			return true;

		debugC(0, kDebugInventory, "lclick on %u:%s", _hoveredObj->id, _hoveredObj->name.c_str());

		if (_selectedObj == NULL) {
			if (tryObjectCallback(_hoveredObj))
				return true;
			//activate(false);
			int w = _vm->res->font7.render(NULL, 0, 0, _hoveredObj->description, textColorMark);
			_vm->scene->displayMessage(_hoveredObj->description, textColorMark, Common::Point((kScreenWidth - w) / 2, 162));
			return true;
		}

		int id1 = _selectedObj->id;
		int id2 = _hoveredObj->id;
		if (id1 == id2)
			return true;

		debugC(0, kDebugInventory, "combine(%u, %u)!", id1, id2);
		byte *table = _vm->res->dseg.ptr(dsAddr_objCombiningTablePtr);
		while (table[0] != 0 && table[1] != 0) {
			if ((id1 == table[0] && id2 == table[1]) || (id2 == table[0] && id1 == table[1])) {
				byte newObj = table[2];
				if (newObj != 0) {
					remove(id1);
					remove(id2);
					debugC(0, kDebugInventory, "adding object %u", newObj);
					add(newObj);
					_vm->playSoundNow(69);
				}
				uint16 msg = READ_LE_UINT16(table + 3);
				_vm->displayMessage(msg);
				activate(false);
				resetSelectedObject();
				return true;
			}
			table += 5;
		}
		_vm->displayMessage(dsAddr_objCombineErrorMsg);
		activate(false);
		resetSelectedObject();
		return true;
	}

	case Common::EVENT_RBUTTONDOWN:
		if (!_active)
			return false;

		if (_hoveredObj != NULL) {
			debugC(0, kDebugInventory, "rclick object %u:%s", _hoveredObj->id, _hoveredObj->name.c_str());
			// do not process callback for banknote on r-click
			if (_hoveredObj->id != kInvItemBanknote && tryObjectCallback(_hoveredObj))
				return true;
		}

		_selectedObj = _hoveredObj;
		if (_selectedObj)
			debugC(0, kDebugInventory, "selected object %s", _selectedObj->name.c_str());
		return true;

	case Common::EVENT_KEYDOWN:
		if (_active && event.kbd.keycode == Common::KEYCODE_ESCAPE) {
			activate(false);
			return true;
		}
		if (event.kbd.keycode == Common::KEYCODE_RETURN) {
			activate(!_active);
			return true;
		}
		return false;

	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONUP:
		return _active;

	default:
		return false;
	}
}

void Inventory::Item::free() {
	_animation.free();
	_surface.free();
}

void Inventory::Item::backgroundEffect(Graphics::Surface *s) {
	uint w = _rect.right - _rect.left, h = _rect.bottom - _rect.top;
	byte *line = (byte *)s->getBasePtr(_rect.left, _rect.top);
	for (uint y = 0; y < h; ++y, line += s->pitch) {
		byte *dst = line;
		for (uint x = 0; x < w; ++x, ++dst) {
			*dst = (*dst == 232) ? 214 : 224;
		}
	}
}

void Inventory::Item::load(Inventory *inventory, uint itemId) {
	InventoryObject *obj = &inventory->_objects[itemId];
	if (obj->animated) {
		if (_animation.empty()) {
			debugC(0, kDebugInventory, "loading item %d from offset %x", obj->id, inventory->_offset[obj->id - 1]);
			Common::MemoryReadStream s(inventory->_items + inventory->_offset[obj->id - 1], inventory->_offset[obj->id] - inventory->_offset[obj->id - 1]);
			_animation.load(s, Animation::kTypeInventory);
		}
	} else {
		if (_surface.empty()) {
			debugC(0, kDebugInventory, "loading item %d from offset %x", obj->id, inventory->_offset[obj->id - 1]);
			Common::MemoryReadStream s(inventory->_items + inventory->_offset[obj->id - 1], inventory->_offset[obj->id] - inventory->_offset[obj->id - 1]);
			_surface.load(s, Surface::kTypeOns);
		}
	}
}

void Inventory::Item::render(Inventory *inventory, uint itemId, Graphics::Surface *dst, int delta) {
	InventoryObject *obj = &inventory->_objects[itemId];

	backgroundEffect(dst);
	_rect.render(dst, _hovered ? 233 : 234);
	load(inventory, itemId);
	if (obj->animated) {
		if (_hovered) {
			Surface *s = _animation.currentFrame(delta);
			if (_animation.currentIndex() == 0)
				s = _animation.currentFrame(1); //force index to be 1 here
			if (s != NULL)
				s->render(dst, _rect.left + 1, _rect.top + 1);
		} else {
			Surface *s = _animation.firstFrame();
			if (s != NULL)
				s->render(dst, _rect.left + 1, _rect.top + 1);
		}
	} else {
		_surface.render(dst, _rect.left + 1, _rect.top + 1);
	}

	Common::String name;
	if (inventory->_selectedObj) {
		name = inventory->_selectedObj->name;
		name += " & ";
	}
	if (inventory->_selectedObj != inventory->_hoveredObj)
		name += obj->name;

	if (_hovered && inventory->_vm->scene->getMessage().empty()) {
		int w = inventory->_vm->res->font7.render(NULL, 0, 0, name, textColorMark, true);
		inventory->_vm->res->font7.render(dst, (kScreenWidth - w) / 2, 180, name, textColorMark, true);
	}
}

void Inventory::render(Graphics::Surface *surface, int delta) {
	if (!_active)
		return;
	debugC(0, kDebugInventory, "Inventory::render()");

	_background.render(surface);

	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 6; x++) {
			int idx = x + 6 * y;
			byte item = _inventory[idx];
			if (item != 0) {
				debugC(0, kDebugInventory, "\t(x, y): %d,%d -> item: %u", x, y, item);
				_graphics[idx].render(this, item, surface, delta);
			}
		}
	}
}

} // End of namespace TeenAgent
