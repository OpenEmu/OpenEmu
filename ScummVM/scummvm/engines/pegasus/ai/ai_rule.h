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

#ifndef PEGASUS_AI_AIRULE_H
#define PEGASUS_AI_AIRULE_H

#include "common/list.h"

#include "pegasus/ai/ai_action.h"
#include "pegasus/ai/ai_condition.h"

namespace Common {
	class ReadStream;
	class WriteStream;
}

namespace Pegasus {

class AICondition;
class AIAction;

class AIRule {
public:
	AIRule(AICondition *condition, AIAction *rule) {
		_ruleCondition = condition;
		_ruleAction = rule;
		_ruleActive = true;
	}

	~AIRule() {
		if (_ruleCondition)
			delete _ruleCondition;

		if (_ruleAction)
			delete _ruleAction;
	}

	bool fireRule();

	void activateRule() { _ruleActive = true; }
	void deactivateRule() { _ruleActive = false; }
	bool isRuleActive() { return _ruleActive; }

	void writeAIRule(Common::WriteStream *);
	void readAIRule(Common::ReadStream *);

protected:
	AICondition *_ruleCondition;
	AIAction *_ruleAction;
	bool _ruleActive;
};

class AIRuleList : public Common::List<AIRule *> {
public:
	AIRuleList() {}
	~AIRuleList() {}

	void writeAIRules(Common::WriteStream *);
	void readAIRules(Common::ReadStream *);
};

} // End of namespace Pegasus

#endif
