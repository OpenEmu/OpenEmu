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

#ifndef LASTEXPRESS_FIGHT_H
#define LASTEXPRESS_FIGHT_H

/*
	Fight structure
	---------------
	uint32 {4}      - player struct
	uint32 {4}      - opponent struct
	uint32 {4}      - hasLost flag

	byte {1}        - isRunning

	Fight participant structure
	---------------------------
	uint32 {4}      - function pointer
	uint32 {4}      - pointer to fight structure
	uint32 {4}      - pointer to opponent (fight participant structure)
	uint32 {4}      - array of sequences
	uint32 {4}      - number of sequences
	uint32 {4}      - ??
	uint32 {4}      - ??
	uint32 {4}      - ??
	uint32 {4}      - ??
	uint32 {4}      - ??
	uint32 {4}      - ??
	uint32 {4}      - ??
	uint32 {4}      - ??
	uint16 {2}      - ??
	uint16 {2}      - ??    - only for opponent structure
	uint32 {4}      - ??    - only for opponent structure

*/

#include "lastexpress/shared.h"

#include "lastexpress/eventhandler.h"

namespace LastExpress {

class LastExpressEngine;
class Sequence;

class Fighter;
class Opponent;

class Fight : public EventHandler {
public:
	enum FightEndType {
		kFightEndWin  = 0,
		kFightEndLost = 1,
		kFightEndExit = 2
	};

	Fight(LastExpressEngine *engine);
	~Fight();

	FightEndType setup(FightType type);

	void eventMouse(const Common::Event &ev);
	void eventTick(const Common::Event &ev);

	// State
	bool isRunning() { return _data->isFightRunning; }
	void setRunningState(bool state) { _data->isFightRunning = state; }
	void bailout(FightEndType type);
	void setStopped();
	void resetState() { _state = 0; }
	void setEndType(FightEndType endType) { _endType = endType; }

private:
	struct FightData {
		Fighter *player;
		Opponent *opponent;
		int32 index;

		Sequence *sequences[20];
		Common::String names[20];

		bool isFightRunning;

		FightData();
		~FightData();
	};

	LastExpressEngine *_engine;
	FightData *_data;
	FightEndType _endType;
	int _state;

	bool _handleTimer;

	// Events
	void handleTick(const Common::Event &ev, bool unknown);

	// Data
	void loadData(FightType type);
	void clearData();
	void setOpponents();
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_FIGHT_H
