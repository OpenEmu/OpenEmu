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

#include "pegasus/ai/ai_action.h"
#include "pegasus/ai/ai_area.h"

namespace Pegasus {

AICompoundAction::~AICompoundAction() {
	for (AIActionList::iterator it = _compoundActions.begin(); it != _compoundActions.end(); it++)
		delete *it;
}

void AICompoundAction::performAIAction(AIRule *rule) {
	for (AIActionList::iterator it = _compoundActions.begin(); it != _compoundActions.end(); it++)
		(*it)->performAIAction(rule);
}

AIPlayMessageAction::AIPlayMessageAction(const Common::String &movieName, bool keepLastFrame, const InputBits interruptionFilter) {
	_movieName = movieName;
	_keepLastFrame = keepLastFrame;
	_interruptionFilter = interruptionFilter;
}

void AIPlayMessageAction::performAIAction(AIRule *) {
	if (g_AIArea) {
		g_AIArea->checkMiddleArea();
		g_AIArea->playAIMovie(kRightAreaSignature, _movieName, _keepLastFrame, _interruptionFilter);
	}
}

AIStartTimerAction::AIStartTimerAction(AITimerCondition *timerCondition) {
	_timerCondition = timerCondition;
}

void AIStartTimerAction::performAIAction(AIRule *) {
	_timerCondition->startTimer();
}

AIActivateRuleAction::AIActivateRuleAction(AIRule *rule) {
	_rule = rule;
}

void AIActivateRuleAction::performAIAction(AIRule *) {
	_rule->activateRule();
}

AIDeactivateRuleAction::AIDeactivateRuleAction(AIRule *rule) {
	_rule = rule;
}

void AIDeactivateRuleAction::performAIAction(AIRule *) {
	_rule->deactivateRule();
}

} // End of namespace Pegasus
