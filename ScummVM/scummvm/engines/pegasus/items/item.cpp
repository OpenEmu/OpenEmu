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

#include "common/error.h"
#include "common/stream.h"

#include "pegasus/constants.h"
#include "pegasus/elements.h"
#include "pegasus/pegasus.h"
#include "pegasus/surface.h"
#include "pegasus/ai/ai_area.h"
#include "pegasus/items/item.h"
#include "pegasus/items/itemlist.h"
#include "pegasus/items/biochips/biochipitem.h"
#include "pegasus/items/inventory/inventoryitem.h"

namespace Pegasus {

Item::Item(const ItemID id, const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant direction) : IDObject(id) {
	_itemNeighborhood = neighborhood;
	_itemRoom = room;
	_itemDirection = direction;
	_itemWeight = 1;
	_itemOwnerID = kNoActorID;
	_itemState = 0;

	PegasusEngine *vm = (PegasusEngine *)g_engine;

	Common::SeekableReadStream *info = vm->_resFork->getResource(kItemInfoResType, kItemBaseResID + id);
	if (info) {
		_itemInfo.infoLeftTime = info->readUint32BE();
		_itemInfo.infoRightStart = info->readUint32BE();
		_itemInfo.infoRightStop = info->readUint32BE();
		_itemInfo.dragSpriteNormalID = info->readUint16BE();
		_itemInfo.dragSpriteUsedID = info->readUint16BE();

		if (vm->isDemo()) {
			// Adjust info right times to account for the stuff that was chopped out of the
			// info right movies.
			// Assumes time scale of 600.

			// Gap times in seconds
			static const TimeValue kGap1 = 24;
			static const TimeValue kGap2 = 34;
			static const TimeValue kGap3 = 4;
			static const TimeValue kGap4 = 4;

			static const TimeValue kGapForGroup1 = kGap1;
			static const TimeValue kGapForGroup2 = kGapForGroup1 + kGap2;
			static const TimeValue kGapForGroup3 = kGapForGroup2 + kGap3;
			static const TimeValue kGapForGroup4 = kGapForGroup3 + kGap4;

			switch (id) {
			case kHistoricalLog:
			case kJourneymanKey:
			case kKeyCard:
				_itemInfo.infoRightStart -= 600 * kGapForGroup1;
				_itemInfo.infoRightStop -= 600 * kGapForGroup1;
				break;
			case kAIBiochip:
				_itemInfo.infoRightStart -= 600 * kGapForGroup2;
				_itemInfo.infoRightStop -= 600 * kGapForGroup2;
				break;
			case kMapBiochip:
				_itemInfo.infoRightStart -= 600 * kGapForGroup3;
				_itemInfo.infoRightStop -= 600 * kGapForGroup3;
				break;
			case kPegasusBiochip:
				_itemInfo.infoRightStart -= 600 * kGapForGroup4;
				_itemInfo.infoRightStop -= 600 * kGapForGroup4;
				break;
			}
		}

		delete info;
	} else {
		memset(&_itemInfo, 0, sizeof(_itemInfo));
	}

	Common::SeekableReadStream *middleAreaInfo = vm->_resFork->getResource(kMiddleAreaInfoResType, kItemBaseResID + id);
	if (middleAreaInfo) {
		_sharedAreaInfo = readItemState(middleAreaInfo);
		delete middleAreaInfo;
	} else {
		// Only kArgonPickup does not have this
		memset(&_sharedAreaInfo, 0, sizeof(_sharedAreaInfo));
	}

	Common::SeekableReadStream *extraInfo = vm->_resFork->getResource(kItemExtraInfoResType, kItemBaseResID + id);
	if (!extraInfo)
		error("Extra info not found for item %d", id);

	_itemExtras.numEntries = extraInfo->readUint16BE();
	_itemExtras.entries = new ItemExtraEntry[_itemExtras.numEntries];
	for (uint16 i = 0; i < _itemExtras.numEntries; i++) {
		_itemExtras.entries[i].extraID = extraInfo->readUint32BE();
		_itemExtras.entries[i].extraArea = extraInfo->readUint16BE();
		_itemExtras.entries[i].extraStart = extraInfo->readUint32BE();
		_itemExtras.entries[i].extraStop = extraInfo->readUint32BE();
	}

	delete extraInfo;

	g_allItems.push_back(this);
}

Item::~Item() {
	delete[] _sharedAreaInfo.entries;
	delete[] _itemExtras.entries;
}

void Item::writeToStream(Common::WriteStream *stream) {
	stream->writeUint16BE(_itemNeighborhood);
	stream->writeUint16BE(_itemRoom);
	stream->writeByte(_itemDirection);
	stream->writeUint16BE(_itemOwnerID);
	stream->writeUint16BE(_itemState);
}

void Item::readFromStream(Common::ReadStream *stream) {
	_itemNeighborhood = stream->readUint16BE();
	_itemRoom = stream->readUint16BE();
	_itemDirection = stream->readByte();
	_itemOwnerID = stream->readUint16BE();
	_itemState = stream->readUint16BE();
}

ActorID Item::getItemOwner() const {
	return _itemOwnerID;
}

void Item::setItemOwner(const ActorID owner) {
	_itemOwnerID = owner;

	if (owner == kNoActorID) {
		if (isSelected())
			deselect();
		removedFromInventory();
	} else {
		addedToInventory();
	}
}

WeightType Item::getItemWeight() {
	return _itemWeight;
}

ItemState Item::getItemState() const {
	return _itemState;
}

void Item::setItemState(const ItemState state) {
	if (state != _itemState) {
		_itemState = state;

		if (getItemType() == kInventoryItemType && ((PegasusEngine *)g_engine)->getCurrentInventoryItem() == (InventoryItem *)this)
			select();
		else if (getItemType() == kBiochipItemType && ((PegasusEngine *)g_engine)->getCurrentBiochip() == (BiochipItem *)this)
			select();
	}
}

void Item::getItemRoom(NeighborhoodID &neighborhood, RoomID &room, DirectionConstant &direction) const {
	neighborhood = _itemNeighborhood;
	room = _itemRoom;
	direction = _itemDirection;
}

void Item::setItemRoom(const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant direction) {
	_itemNeighborhood = neighborhood;
	_itemRoom = room;
	_itemDirection = direction;

	if (neighborhood == kNoNeighborhoodID)
		pickedUp();
	else
		dropped();
}

NeighborhoodID Item::getItemNeighborhood() const {
	return _itemNeighborhood;
}

TimeValue Item::getSharedAreaTime() const {
	if (!_sharedAreaInfo.entries)
		return 0xffffffff;

	TimeValue time;
	ItemState state;

	findItemStateEntryByState(_sharedAreaInfo, _itemState, time);
	if (time == 0xffffffff)
		getItemStateEntry(_sharedAreaInfo, 0, state, time);

	return time;
}

// Must affect images in shared area.
void Item::select() {
	_isSelected = true;

	if (g_AIArea) {
		if (getItemType() == kInventoryItemType)
			g_AIArea->setAIAreaToTime(kInventorySignature, kMiddleAreaSignature, getSharedAreaTime());
		else
			g_AIArea->setAIAreaToTime(kBiochipSignature, kMiddleAreaSignature, getSharedAreaTime());
	}
}

void Item::deselect() {
	_isActive = false;
	_isSelected = false;

	if (g_AIArea) {
		if (getItemType() == kInventoryItemType)
			g_AIArea->setAIAreaToTime(kInventorySignature, kMiddleAreaSignature, 0xffffffff);
		else
			g_AIArea->setAIAreaToTime(kBiochipSignature, kMiddleAreaSignature, 0xffffffff);
	}
}

void Item::getItemStateEntry(ItemStateInfo info, uint32 index, ItemState &state, TimeValue &time) {
	if (index < info.numEntries) {
		state = info.entries[index].itemState;
		time = info.entries[index].itemTime;
	} else {
		state = kNoItemState;
		time = 0xffffffff;
	}
}

void Item::findItemStateEntryByState(ItemStateInfo info, ItemState state, TimeValue &time) {
	for (uint16 i = 0; i < info.numEntries; i++) {
		if (info.entries[i].itemState == state) {
			time = info.entries[i].itemTime;
			return;
		}
	}

	time = 0xffffffff;
}

ItemStateInfo Item::readItemState(Common::SeekableReadStream *stream) {
	ItemStateInfo info;

	info.numEntries = stream->readUint16BE();
	info.entries = new ItemStateEntry[info.numEntries];
	for (uint16 i = 0; i < info.numEntries; i++) {
		info.entries[i].itemState = stream->readSint16BE();
		info.entries[i].itemTime = stream->readUint32BE();
	}

	return info;
}

Sprite *Item::getDragSprite(const DisplayElementID id) const {
	PegasusEngine *vm = (PegasusEngine *)g_engine;
	Sprite *result = new Sprite(id);
	SpriteFrame *frame = new SpriteFrame();

	frame->initFromPICTResource(vm->_resFork, _itemInfo.dragSpriteNormalID, true);
	result->addFrame(frame, 0, 0);

	if (_itemInfo.dragSpriteNormalID != _itemInfo.dragSpriteUsedID) {
		frame = new SpriteFrame();
		frame->initFromPICTResource(vm->_resFork, _itemInfo.dragSpriteUsedID, true);
	}

	result->addFrame(frame, 0, 0);
	result->setCurrentFrameIndex(0);
	return result;
}

TimeValue Item::getInfoLeftTime() const {
	return _itemInfo.infoLeftTime;
}

void Item::getInfoRightTimes(TimeValue &start, TimeValue &stop) const {
	start = _itemInfo.infoRightStart;
	stop = _itemInfo.infoRightStop;
}

void Item::findItemExtra(const uint32 extraID, ItemExtraEntry &entry) {
	for (uint32 i = 0; i < _itemExtras.numEntries; i++) {
		if (_itemExtras.entries[i].extraID == extraID) {
			entry = _itemExtras.entries[i];
			return;
		}
	}
}

} // End of namespace Pegasus
