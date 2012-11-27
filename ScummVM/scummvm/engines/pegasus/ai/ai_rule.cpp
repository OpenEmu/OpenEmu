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

#include "common/stream.h"

#include "pegasus/ai/ai_action.h"
#include "pegasus/ai/ai_area.h"
#include "pegasus/ai/ai_condition.h"
#include "pegasus/ai/ai_rule.h"

namespace Pegasus {

bool AIRule::fireRule() {
	if (_ruleActive && _ruleCondition && _ruleAction && _ruleCondition->fireCondition()) {
		if (g_AIArea)
			g_AIArea->lockAIOut();

		_ruleAction->performAIAction(this);

		if (--_ruleAction->_actionCount <= 0)
			deactivateRule();

		if (g_AIArea)
			g_AIArea->unlockAI();

		return true;
	}

	return false;
}

void AIRule::writeAIRule(Common::WriteStream *stream) {
	stream->writeByte(_ruleActive);

	if (_ruleCondition)
		_ruleCondition->writeAICondition(stream);
}

void AIRule::readAIRule(Common::ReadStream *stream) {
	_ruleActive = stream->readByte();

	if (_ruleCondition)
		_ruleCondition->readAICondition(stream);
}

void AIRuleList::writeAIRules(Common::WriteStream *stream) {
	for (AIRuleList::iterator it = begin(); it != end(); it++)
		(*it)->writeAIRule(stream);
}

void AIRuleList::readAIRules(Common::ReadStream *stream) {
	for (AIRuleList::iterator it = begin(); it != end(); it++)
		(*it)->readAIRule(stream);
}

} // End of namespace Pegasus
