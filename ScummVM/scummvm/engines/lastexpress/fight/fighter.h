/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#ifndef LASTEXPRESS_FIGHTER_H
#define LASTEXPRESS_FIGHTER_H

#include "lastexpress/fight/fight.h"

#include "common/array.h"

namespace LastExpress {

class Fight;
class Sequence;
class SequenceFrame;

class Fighter {
public:
	enum FightAction {
		kFightActionNone       = 0,
		kFightAction1          = 1,
		kFightAction2          = 2,
		kFightAction3          = 3,
		kFightAction4          = 4,
		kFightAction5          = 5,
		kFightAction101        = 101,
		kFightActionResetFrame = 102,
		kFightAction103        = 103,
		kFightActionWin        = 104,
		kFightActionLost       = 105,
		kFightAction128        = 128,
		kFightAction129        = 129,
		kFightAction130        = 130,
		kFightAction131        = 131,
		kFightAction132        = 132
	};

	enum FightSequenceType {
		kFightSequenceType0 = 0,
		kFightSequenceType1 = 1,
		kFightSequenceType2 = 2
	};

	Fighter(LastExpressEngine *engine);
	virtual ~Fighter();

	// Default functions
	virtual void handleAction(FightAction action);
	virtual void update();
	virtual bool canInteract(FightAction action = kFightActionNone);

	// Drawing
	void setSequenceAndDraw(uint32 sequenceIndex, FightSequenceType type);

	// Accessors
	void setOpponent(Fighter *opponent) { _opponent = opponent; }
	void setCountdown(int32 countdown) { _countdown = countdown; }
	void setFight(Fight *fight) { _fight = fight; }

	int getCountdown() { return _countdown; }
	uint32 getSequenceIndex() { return _sequenceIndex; }
	uint32 getField34() { return _field_34; }

protected:
	LastExpressEngine         *_engine;
	Fight                     *_fight;
	Fighter                   *_opponent;
	Sequence                  *_sequence;
	SequenceFrame             *_frame;
	uint32                     _sequenceIndex;
	Common::Array<Sequence *>  _sequences;
	uint32                     _frameIndex;
	uint32                     _field_24;
	FightAction                _action;
	uint32                     _sequenceIndex2;
	int32                      _countdown;  // countdown before loosing ?
	uint32                     _field_34;

	// Drawing and processing
	void draw();
	void process();

	// Helpers
	bool checkFrame(uint32 val);
};

class Opponent : public Fighter {
public:
	Opponent(LastExpressEngine *engine) : Fighter(engine) {
		_field_38 = 0;
	}

	virtual void update();

protected:
	int32 _field_38;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_FIGHTER_H
