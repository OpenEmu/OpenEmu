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

#ifndef PEGASUS_AI_AICONDITION_H
#define PEGASUS_AI_AICONDITION_H

#include "pegasus/timers.h"

namespace Common {
	class ReadStream;
	class WriteStream;
}

namespace Pegasus {

/////////////////////////////////////////////
//
// AICondition

class AICondition {
public:
	AICondition() {}
	virtual ~AICondition() {}

	virtual bool fireCondition() = 0;

	// Only need these for conditions that are dynamic, like timer conditions...
	// other conditions, like item related conditions, which don't change during
	// the run of an environment, are completely initted when the environment
	// is created.
	virtual void writeAICondition(Common::WriteStream *) {}
	virtual void readAICondition(Common::ReadStream *) {}
};

/////////////////////////////////////////////
//
// AIOneChildCondition

class AIOneChildCondition : public AICondition {
public:
	AIOneChildCondition(AICondition *);
	virtual ~AIOneChildCondition();

	virtual void writeAICondition(Common::WriteStream *);
	virtual void readAICondition(Common::ReadStream *);

protected:
	AICondition *_child;
};

/////////////////////////////////////////////
//
// AITwoChildrenCondition

class AITwoChildrenCondition : public AICondition {
public:
	AITwoChildrenCondition(AICondition *, AICondition *);
	virtual ~AITwoChildrenCondition();

	virtual void writeAICondition(Common::WriteStream *);
	virtual void readAICondition(Common::ReadStream *);

protected:
	AICondition *_leftChild, *_rightChild;
};

/////////////////////////////////////////////
//
// AINotCondition

class AINotCondition : public AIOneChildCondition {
public:
	AINotCondition(AICondition *);

	virtual bool fireCondition();
};

/////////////////////////////////////////////
//
// AIAndCondition

class AIAndCondition : public AITwoChildrenCondition {
public:
	AIAndCondition(AICondition *, AICondition *);

	virtual bool fireCondition();
};

/////////////////////////////////////////////
//
// AIOrCondition

class AIOrCondition : public AITwoChildrenCondition {
public:
	AIOrCondition(AICondition *, AICondition *);

	virtual bool fireCondition();
};

/////////////////////////////////////////////
//
// AITimerCondition

class AITimerCondition : public AICondition {
public:
	AITimerCondition(const TimeValue, const TimeScale, const bool);

	void startTimer();
	void stopTimer();

	virtual bool fireCondition();

	virtual void writeAICondition(Common::WriteStream *);
	virtual void readAICondition(Common::ReadStream *);

protected:
	void fire();

	FuseFunction _timerFuse;
	bool _fired;
};

/////////////////////////////////////////////
//
// AILocationCondition

class AILocationCondition : public AICondition {
public:
	AILocationCondition(uint32);
	virtual ~AILocationCondition();

	void addLocation(RoomViewID);
	virtual bool fireCondition();

	virtual void writeAICondition(Common::WriteStream *);
	virtual void readAICondition(Common::ReadStream *);

protected:
	uint32 _numLocations, _maxLocations;
	RoomViewID *_locations;
};

/////////////////////////////////////////////
//
// AIDoorOpenedCondition

class AIDoorOpenedCondition : public AICondition {
public:
	AIDoorOpenedCondition(RoomViewID);
	virtual ~AIDoorOpenedCondition() {}

	virtual bool fireCondition();

protected:
	RoomViewID _doorLocation;
};

/////////////////////////////////////////////
//
// AIHasItemCondition

class AIHasItemCondition : public AICondition {
public:
	AIHasItemCondition(const ItemID);

	virtual bool fireCondition();

protected:
	ItemID _item;
};

/////////////////////////////////////////////
//
// AIDoesntHaveItemCondition

class AIDoesntHaveItemCondition : public AICondition {
public:
	AIDoesntHaveItemCondition(const ItemID);

	virtual bool fireCondition();

protected:
	ItemID _item;
};

/////////////////////////////////////////////
//
// AICurrentItemCondition

class AICurrentItemCondition : public AICondition {
public:
	AICurrentItemCondition(const ItemID);

	virtual bool fireCondition();

protected:
	ItemID _item;
};

/////////////////////////////////////////////
//
// AICurrentBiochipCondition

class AICurrentBiochipCondition : public AICondition {
public:
	AICurrentBiochipCondition(const ItemID);

	virtual bool fireCondition();

protected:
	ItemID _biochip;
};

/////////////////////////////////////////////
//
// AIItemStateCondition

class AIItemStateCondition : public AICondition {
public:
	AIItemStateCondition(const ItemID, const ItemState);

	virtual bool fireCondition();

protected:
	ItemID _item;
	ItemState _state;
};

/////////////////////////////////////////////
//
// AIEnergyMonitorCondition

class AIEnergyMonitorCondition : public AICondition {
public:
	AIEnergyMonitorCondition(const int32);

	virtual bool fireCondition();

protected:
	int32 _energyThreshold;
};

/////////////////////////////////////////////
//
// AILastExtraCondition

class AILastExtraCondition : public AICondition {
public:
	AILastExtraCondition(const ExtraID);

	virtual bool fireCondition();

protected:
	ExtraID _lastExtra;
};

/////////////////////////////////////////////
//
// Helper functions

AICondition *makeLocationAndDoesntHaveItemCondition(const RoomID room, const DirectionConstant direction, const ItemID item);

} // End of namespace Pegasus

#endif
