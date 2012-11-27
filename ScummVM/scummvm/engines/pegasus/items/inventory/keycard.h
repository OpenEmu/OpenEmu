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

#ifndef PEGASUS_ITEMS_INVENTORY_KEYCARD_H
#define PEGASUS_ITEMS_INVENTORY_KEYCARD_H

#include "pegasus/items/inventory/inventoryitem.h"

namespace Pegasus {

class KeyCard : public InventoryItem {
public:
	KeyCard(const ItemID, const NeighborhoodID, const RoomID, const DirectionConstant);
	virtual ~KeyCard() {}

	virtual void toggleItemState();
	virtual void setItemState(const ItemState);
	bool isFlashlightOn();

protected:
	virtual void removedFromInventory();
};

} // End of namespace Pegasus

#endif
