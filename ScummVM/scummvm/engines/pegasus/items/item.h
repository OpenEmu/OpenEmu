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

#ifndef PEGASUS_ITEMS_ITEM_H
#define PEGASUS_ITEMS_ITEM_H

#include "common/endian.h"

#include "pegasus/types.h"
#include "pegasus/util.h"

namespace Common {
	class Error;
	class ReadStream;
	class WriteStream;
	class SeekableReadStream;
}

namespace Pegasus {

// JMPItemInfo contains resource data used by all Items.

struct JMPItemInfo {
	TimeValue infoLeftTime;
	TimeValue infoRightStart;
	TimeValue infoRightStop;
	uint32 dragSpriteNormalID;
	uint32 dragSpriteUsedID;
};

// ItemStateEntry contains a single state/TimeValue pair. The TimeValue is
// the time value to set the shared area movie that corresponds with the given
// state of an inventory item.

struct ItemStateEntry {
	ItemState itemState;
	TimeValue itemTime;
};

struct ItemStateInfo {
	uint16 numEntries; // For easy ResEdit access
	ItemStateEntry *entries;
};

// ItemExtraEntry

static const short kLeftAreaExtra = 0;
static const short kMiddleAreaExtra = 1;
static const short kRightAreaExtra = 2;

struct ItemExtraEntry {
	uint32 extraID;
	uint16 extraArea;
	TimeValue extraStart;
	TimeValue extraStop;
};

struct ItemExtraInfo {
	uint16 numEntries; // For easy ResEdit access
	ItemExtraEntry *entries;
};

// Inventory info resource type and ID:
// Individual inventory items are stored in these resource types.
// Resource ID is item ID + kItemBaseResID.

static const uint32 kItemInfoResType = MKTAG('I', 't', 'e', 'm');       // JMPItemInfoHandle
static const uint32 kLeftAreaInfoResType = MKTAG('L', 'e', 'f', 't');   // ItemStateInfoHandle
static const uint32 kMiddleAreaInfoResType = MKTAG('M', 'i', 'd', 'l'); // ItemStateInfoHandle
static const uint32 kRightAreaInfoResType = MKTAG('R', 'g', 'h', 't');  // ItemStateInfoHandle
static const uint32 kItemExtraInfoResType = MKTAG('I', 'X', 't', 'r');	 // ItemExtraInfoHandle

static const uint16 kItemBaseResID = 128;

// Item IDs.

static const ItemID kAirMask = 7;
static const ItemID kAntidote = 8;
static const ItemID kArgonCanister = 9;
static const ItemID kCardBomb = 10;
static const ItemID kCrowbar = 11;
static const ItemID kGasCanister = 12;
static const ItemID kHistoricalLog = 13;
static const ItemID kJourneymanKey = 14;
static const ItemID kKeyCard = 15;
static const ItemID kMachineGun = 16;
static const ItemID kMarsCard = 17;
static const ItemID kNitrogenCanister = 18;
static const ItemID kOrangeJuiceGlassFull = 19;
static const ItemID kOrangeJuiceGlassEmpty = 20;
static const ItemID kPoisonDart = 21;
static const ItemID kSinclairKey = 22;
static const ItemID kStunGun = 23;
static const ItemID kArgonPickup = 24;

// Biochips.

static const ItemID kAIBiochip = 0;
static const ItemID kInterfaceBiochip = 1;
static const ItemID kMapBiochip = 2;
static const ItemID kOpticalBiochip = 3;
static const ItemID kPegasusBiochip = 4;
static const ItemID kRetinalScanBiochip = 5;
static const ItemID kShieldBiochip = 6;

static const ItemID kNumItems = 25;

// Item States.

static const ItemState kAI000 = 0;
static const ItemState kAI005 = 1;
static const ItemState kAI006 = 2;
static const ItemState kAI010 = 3;
static const ItemState kAI015 = 4;
static const ItemState kAI016 = 5;
static const ItemState kAI020 = 6;
static const ItemState kAI024 = 7;
static const ItemState kAI100 = 8;
static const ItemState kAI101 = 9;
static const ItemState kAI105 = 10;
static const ItemState kAI106 = 11;
static const ItemState kAI110 = 12;
static const ItemState kAI111 = 13;
static const ItemState kAI115 = 14;
static const ItemState kAI116 = 15;
static const ItemState kAI120 = 16;
static const ItemState kAI121 = 17;
static const ItemState kAI124 = 18;
static const ItemState kAI125 = 19;
static const ItemState kAI126 = 20;
static const ItemState kAI200 = 21;
static const ItemState kAI201 = 22;
static const ItemState kAI202 = 23;
static const ItemState kAI205 = 24;
static const ItemState kAI206 = 25;
static const ItemState kAI210 = 26;
static const ItemState kAI211 = 27;
static const ItemState kAI212 = 28;
static const ItemState kAI215 = 29;
static const ItemState kAI216 = 30;
static const ItemState kAI220 = 31;
static const ItemState kAI221 = 32;
static const ItemState kAI222 = 33;
static const ItemState kAI224 = 34;
static const ItemState kAI225 = 35;
static const ItemState kAI226 = 36;
static const ItemState kAI300 = 37;
static const ItemState kAI301 = 38;
static const ItemState kAI302 = 39;
static const ItemState kAI303 = 40;
static const ItemState kAI305 = 41;
static const ItemState kAI306 = 42;
static const ItemState kAI310 = 43;
static const ItemState kAI311 = 44;
static const ItemState kAI312 = 45;
static const ItemState kAI313 = 46;
static const ItemState kAI315 = 47;
static const ItemState kAI316 = 48;
static const ItemState kAI320 = 49;
static const ItemState kAI321 = 50;
static const ItemState kAI322 = 51;
static const ItemState kAI323 = 52;
static const ItemState kAI324 = 53;
static const ItemState kAI325 = 54;
static const ItemState kAI326 = 55;
static const ItemState kNormalItem = 56;
static const ItemState kMapUnavailable = 57;
static const ItemState kMapEngaged = 58;
static const ItemState kOptical000 = 59;
static const ItemState kOptical001 = 60;
static const ItemState kOptical002 = 61;
static const ItemState kOptical010 = 62;
static const ItemState kOptical011 = 63;
static const ItemState kOptical012 = 64;
static const ItemState kOptical020 = 65;
static const ItemState kOptical021 = 66;
static const ItemState kOptical100 = 67;
static const ItemState kOptical101 = 68;
static const ItemState kOptical102 = 69;
static const ItemState kOptical110 = 70;
static const ItemState kOptical111 = 71;
static const ItemState kOptical112 = 72;
static const ItemState kOptical120 = 73;
static const ItemState kOptical121 = 74;
static const ItemState kOptical200 = 75;
static const ItemState kOptical201 = 76;
static const ItemState kOptical210 = 77;
static const ItemState kOptical211 = 78;
static const ItemState kPegasusTSA00 = 79;
static const ItemState kPegasusTSA10 = 80;
static const ItemState kPegasusPrehistoric00 = 81;
static const ItemState kPegasusPrehistoric01 = 82;
static const ItemState kPegasusPrehistoric10 = 83;
static const ItemState kPegasusPrehistoric11 = 84;
static const ItemState kPegasusMars00 = 85;
static const ItemState kPegasusMars01 = 86;
static const ItemState kPegasusMars10 = 87;
static const ItemState kPegasusMars11 = 88;
static const ItemState kPegasusNorad00 = 89;
static const ItemState kPegasusNorad01 = 90;
static const ItemState kPegasusNorad10 = 91;
static const ItemState kPegasusNorad11 = 92;
static const ItemState kPegasusWSC00 = 93;
static const ItemState kPegasusWSC01 = 94;
static const ItemState kPegasusWSC10 = 95;
static const ItemState kPegasusWSC11 = 96;
static const ItemState kPegasusCaldoria = 97;
static const ItemState kRetinalSimulating = 98;
static const ItemState kShieldNormal = 99;
static const ItemState kShieldRadiation = 100;
static const ItemState kShieldPlasma = 101;
static const ItemState kShieldCardBomb = 102;
static const ItemState kShieldDraining = 103;
static const ItemState kAirMaskEmptyOff = 104;
static const ItemState kAirMaskEmptyFilter = 105;
static const ItemState kAirMaskLowOff = 106;
static const ItemState kAirMaskLowFilter = 107;
static const ItemState kAirMaskLowOn = 108;
static const ItemState kAirMaskFullOff = 109;
static const ItemState kAirMaskFullFilter = 110;
static const ItemState kAirMaskFullOn = 111;
static const ItemState kArgonEmpty = 112;
static const ItemState kArgonFull = 113;
static const ItemState kFlashlightOff = 114;
static const ItemState kFlashlightOn = 115;
static const ItemState kNitrogenEmpty = 116;
static const ItemState kNitrogenFull = 117;
static const ItemState kFullGlass = 118;

// Extra IDs.

static const uint32 kRetinalScanSearching = 0;
static const uint32 kRetinalScanActivated = 1;
static const uint32 kShieldIntro = 2;
static const uint32 kRemoveAirMask = 3;
static const uint32 kRemoveArgon = 4;
static const uint32 kRemoveCrowbar = 5;
static const uint32 kGasCanLoop = 6;
static const uint32 kRemoveJourneymanKey = 7;
static const uint32 kRemoveMarsCard = 8;
static const uint32 kRemoveNitrogen = 9;
static const uint32 kRemoveGlass = 10;
static const uint32 kRemoveDart = 11;
static const uint32 kRemoveSinclairKey = 12;

enum ItemType {
	kInventoryItemType,
	kBiochipItemType
};

class Sprite;

/*

	Item is an object which can be picked up and carried around.
	Items have
		a location
		an ID.
		weight
		an owner (kNoActorID if no one is carrying the Item)

*/

class Item : public IDObject {
public:
	Item(const ItemID id, const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant direction);
	virtual ~Item();

	// WriteToStream writes everything EXCEPT the item's ID.
	// It is assumed that the calling function will write and read the ID.
	virtual void writeToStream(Common::WriteStream *stream);
	virtual void readFromStream(Common::ReadStream *stream);

	virtual ActorID getItemOwner() const;
	virtual void setItemOwner(const ActorID owner);

	void getItemRoom(NeighborhoodID &neighborhood, RoomID &room, DirectionConstant &direction) const;
	void setItemRoom(const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant direction);
	NeighborhoodID getItemNeighborhood() const;

	virtual WeightType getItemWeight();

	virtual void setItemState(const ItemState state);
	virtual ItemState getItemState() const;

	virtual ItemType getItemType() = 0;

	TimeValue getInfoLeftTime() const;
	void getInfoRightTimes(TimeValue &, TimeValue &) const;
	TimeValue getSharedAreaTime() const;

	Sprite *getDragSprite(const DisplayElementID) const;

	/*
		select		--	called when this item becomes current. Also called when the inventory
						panel holding this item is raised and this is the current item.
		deselect	--	called when this item is no longer current.
		activate	--	called on the current item when the panel is closed.
	*/
	// In an override of these three member functions, you must call the inherited
	// member functions.
	virtual void select();
	virtual void deselect();
	virtual bool isSelected() { return _isSelected; }

	virtual void activate() { _isActive = true; }
	virtual bool isActive() { return _isActive; }
	virtual void pickedUp() {}
	virtual void addedToInventory() {}
	virtual void removedFromInventory() {}
	virtual void dropped() {}

	// Called when the shared area is taken by another item, but this item is still
	// selected.
	virtual void giveUpSharedArea() {}
	virtual void takeSharedArea() {}

	void findItemExtra(const uint32 extraID, ItemExtraEntry &entry);

protected:
	NeighborhoodID _itemNeighborhood;
	RoomID	_itemRoom;
	DirectionConstant _itemDirection;
	ActorID _itemOwnerID;
	WeightType _itemWeight;
	ItemState _itemState;

	JMPItemInfo _itemInfo;
	ItemStateInfo _sharedAreaInfo;
	ItemExtraInfo _itemExtras;
	bool _isActive;
	bool _isSelected;

	static void getItemStateEntry(ItemStateInfo, uint32, ItemState &, TimeValue &);
	static void findItemStateEntryByState(ItemStateInfo, ItemState, TimeValue &);
	static ItemStateInfo readItemState(Common::SeekableReadStream *stream);
};

} // End of namespace Pegasus

#endif
