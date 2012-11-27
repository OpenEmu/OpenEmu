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

#ifndef PEGASUS_AI_AIACTION_H
#define PEGASUS_AI_AIACTION_H

#include "common/list.h"

#include "pegasus/input.h"
#include "pegasus/types.h"

namespace Pegasus {

class AIRule;
class AITimerCondition;

/////////////////////////////////////////////
//
// AIAction

class AIAction {
friend class AIRule;
public:
	AIAction() { _actionCount = 1; }
	virtual ~AIAction() {}

	virtual void performAIAction(AIRule *) = 0;

	void setActionCount(const uint32 count) { _actionCount = count; }

protected:
	uint32 _actionCount;
};

typedef Common::List<AIAction *> AIActionList;

/////////////////////////////////////////////
//
// AICompoundAction

class AICompoundAction : public AIAction {
public:
	AICompoundAction() {}
	virtual ~AICompoundAction();

	void addAction(AIAction *action) { _compoundActions.push_back(action); }

	virtual void performAIAction(AIRule *);

protected:
	AIActionList _compoundActions;
};

/////////////////////////////////////////////
//
// AIPlayMessageAction

class AIPlayMessageAction : public AIAction {
public:
	AIPlayMessageAction(const Common::String &movieName, bool keepLastFrame, const InputBits = kWarningInterruption);

	virtual void performAIAction(AIRule *);

protected:
	Common::String _movieName;
	InputBits _interruptionFilter;
	bool _keepLastFrame;
};

/////////////////////////////////////////////
//
// AIStartTimerAction

class AIStartTimerAction : public AIAction {
public:
	AIStartTimerAction(AITimerCondition *);

	virtual void performAIAction(AIRule *);

protected:
	AITimerCondition *_timerCondition;
};

/////////////////////////////////////////////
//
// AIActivateRuleAction

class AIActivateRuleAction : public AIAction {
public:
	AIActivateRuleAction(AIRule *);

	virtual void performAIAction(AIRule *);

protected:
	AIRule *_rule;
};

/////////////////////////////////////////////
//
// AIDeactivateRuleAction

class AIDeactivateRuleAction : public AIAction {
public:
	AIDeactivateRuleAction(AIRule *rule);

	virtual void performAIAction(AIRule *);

protected:
	AIRule *_rule;
};

} // End of namespace Pegasus

#endif
