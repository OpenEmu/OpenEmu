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

#ifndef PEGASUS_TYPES_H
#define PEGASUS_TYPES_H

#include "common/scummsys.h"

namespace Pegasus {

// TODO: Probably all of these don't really need to be typedef'd...

typedef int32 DisplayElementID;
typedef int32 DisplayOrder;

typedef int16 HotSpotID;
typedef uint32 HotSpotFlags;

typedef byte ButtonState;
typedef uint32 InputBits;

typedef int32 NotificationID;
typedef uint32 NotificationFlags;

// Mac types.
typedef int16 ResIDType;
typedef int16 CoordType;

enum SlideDirection {
	kSlideLeftMask = 1,
	kSlideRightMask = kSlideLeftMask << 1,
	kSlideUpMask = kSlideRightMask << 1 << 1,
	kSlideDownMask = kSlideUpMask << 1,

	kSlideHorizMask = kSlideLeftMask | kSlideRightMask,
	kSlideVertMask = kSlideUpMask | kSlideDownMask,

	kSlideUpLeftMask = kSlideLeftMask | kSlideUpMask,
	kSlideUpRightMask = kSlideRightMask | kSlideUpMask,
	kSlideDownLeftMask = kSlideLeftMask | kSlideDownMask,
	kSlideDownRightMask = kSlideRightMask | kSlideDownMask
};

// ScummVM QuickTime/QuickDraw replacement types
typedef uint TimeValue;
typedef uint TimeScale;

typedef int16 GameID;

typedef GameID ItemID;
typedef GameID ActorID;
typedef GameID RoomID;
typedef GameID NeighborhoodID;
typedef byte AlternateID;
typedef int8 HotSpotActivationID;

typedef int16 WeightType;

typedef byte DirectionConstant;
typedef byte TurnDirection;

// Meant to be room in low 16 bits and direction in high 16 bits.
typedef uint32 RoomViewID;

#define MakeRoomView(room, direction) (((RoomViewID) (room)) | (((RoomViewID) (direction)) << 16))

typedef uint32 ExtraID;

typedef int16 GameMode;

typedef int16 WeightType;

typedef int16 ItemState;

typedef int8 DeathReason;

typedef int32 GameMenuCommand;

typedef int32 GameScoreType;

typedef long CanMoveForwardReason;

typedef long CanTurnReason;

typedef long CanOpenDoorReason;

enum InventoryResult {
	kInventoryOK,
	kTooMuchWeight,
	kItemNotInInventory
};

typedef int32 InteractionID;

typedef int32 AIConditionID;

enum EnergyStage {
	kStageNoStage,
	kStageCasual,       // more than 50% energy
	kStageWorried,      // more than 25% energy
	kStageNervous,      // more than 5% energy
	kStagePanicStricken // less than 5% energy
};

enum NoradSubPrepState {
	kSubNotPrepped,
	kSubPrepped,
	kSubDamaged
};

enum LowerClientSignature {
	kNoClientSignature,
	kInventorySignature,
	kBiochipSignature,
	kAISignature
};

enum LowerAreaSignature {
	kLeftAreaSignature,
	kMiddleAreaSignature,
	kRightAreaSignature
};

enum AirQuality {
	kAirQualityGood,
	kAirQualityDirty,
	kAirQualityVacuum
};

enum DragType {
	kDragNoDrag,
	kDragInventoryPickup,
	kDragBiochipPickup,
	kDragInventoryUse
};

} // End of namespace Pegasus

#endif
