/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#include "pegasus/constants.h"
#include "pegasus/items/item.h"
#include "pegasus/items/inventory.h"

namespace Pegasus {

Inventory::Inventory() {
	_weightLimit = 100;
	_ownerID = kNoActorID;
	_referenceCount = 0;
}

Inventory::~Inventory() {
}

void Inventory::setWeightLimit(WeightType limit) {
	_weightLimit = limit;
	// *** What to do if the new weight limit is greater than the current weight?
}

WeightType Inventory::getWeight() {
	WeightType	result = 0;

	for (ItemIterator it = _inventoryList.begin(); it != _inventoryList.end(); it++)
		result += (*it)->getItemWeight();

	return result;
}

// If the item already belongs, just return kInventoryOK.
InventoryResult Inventory::addItem(Item *item) {
	if (itemInInventory(item))
		return kInventoryOK;

	if (getWeight() + item->getItemWeight() > _weightLimit)
		return kTooMuchWeight;

	_inventoryList.push_back(item);
	item->setItemOwner(_ownerID);

	++_referenceCount;
	return kInventoryOK;
}

InventoryResult Inventory::removeItem(Item *item) {
	for (ItemIterator it = _inventoryList.begin(); it != _inventoryList.end(); it++) {
		if (*it == item) {
			_inventoryList.erase(it);
			item->setItemOwner(kNoActorID);

			++_referenceCount;
			return kInventoryOK;
		}
	}

	return kItemNotInInventory;
}

InventoryResult Inventory::removeItem(ItemID id) {
	Item *item = findItemByID(id);

	if (item) {
		_inventoryList.remove(item);
		item->setItemOwner(kNoActorID);

		++_referenceCount;
		return kInventoryOK;
	}

	return kItemNotInInventory;
}

void Inventory::removeAllItems() {
	_inventoryList.clear();
	++_referenceCount;
}

bool Inventory::itemInInventory(Item *item) {
	for (ItemIterator it = _inventoryList.begin(); it != _inventoryList.end(); it++)
		if (*it == item)
			return true;

	return false;
}

bool Inventory::itemInInventory(ItemID id) {
	return findItemByID(id) != NULL;
}

Item *Inventory::getItemAt(int32 index) {
	int32 i = 0;
	for (ItemIterator it = _inventoryList.begin(); it != _inventoryList.end(); it++, i++)
		if (i == index)
			return *it;

	return 0;
}

ItemID Inventory::getItemIDAt(int32 index) {
	Item *item = getItemAt(index);

	if (item)
		return item->getObjectID();

	return kNoItemID;
}

Item *Inventory::findItemByID(ItemID id) {
	return _inventoryList.findItemByID(id);
}

// Return -1 if not found.

int32 Inventory::findIndexOf(Item *item) {
	uint32 i = 0;
	for (ItemIterator it = _inventoryList.begin(); it != _inventoryList.end(); it++, i++)
		if (*it == item)
			return i;

	return -1;
}

// Return -1 if not found.

int32 Inventory::findIndexOf(ItemID id) {
	uint32 i = 0;
	for (ItemIterator it = _inventoryList.begin(); it != _inventoryList.end(); it++, i++)
		if ((*it)->getObjectID() == id)
			return i;

	return -1;
}

WeightType Inventory::getWeightLimit() {
	return _weightLimit;
}

int32 Inventory::getNumItems() {
	return _inventoryList.size();
}

void Inventory::setOwnerID(const ActorID id) {
	_ownerID = id;
}

ActorID Inventory::getOwnerID() const {
	return _ownerID;
}

} // End of namespae Pegasus
