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

#include "lastexpress/menu/clock.h"

#include "lastexpress/data/sequence.h"

#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/helpers.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"

namespace LastExpress {

Clock::Clock(LastExpressEngine *engine) : _engine(engine), _frameMinutes(NULL), _frameHour(NULL), _frameSun(NULL), _frameDate(NULL) {
	_frameMinutes = new SequenceFrame(loadSequence("eggmin.seq"), 0, true);
	_frameHour = new SequenceFrame(loadSequence("egghour.seq"), 0, true);
	_frameSun = new SequenceFrame(loadSequence("sun.seq"), 0, true);
	_frameDate = new SequenceFrame(loadSequence("datenew.seq"), 0, true);
}

Clock::~Clock() {
	SAFE_DELETE(_frameMinutes);
	SAFE_DELETE(_frameHour);
	SAFE_DELETE(_frameSun);
	SAFE_DELETE(_frameDate);

	// Zero passed pointers
	_engine = NULL;
}

void Clock::clear() {
	getScenes()->removeFromQueue(_frameMinutes);
	getScenes()->removeFromQueue(_frameHour);
	getScenes()->removeFromQueue(_frameSun);
	getScenes()->removeFromQueue(_frameDate);
}

void Clock::draw(uint32 time) {
	assert(time >= kTimeCityParis && time <= kTimeCityConstantinople);

	// Check that sequences have been loaded
	if (!_frameMinutes || !_frameHour || !_frameSun || !_frameDate)
		error("[Clock::draw] Clock sequences have not been loaded correctly");

	// Clear existing frames
	clear();

	// Game starts at: 1037700 = 7:13 p.m. on July 24, 1914
	// Game ends at:   4941000 = 7:30 p.m. on July 26, 1914
	// Game lasts for: 3903300 = 2 days + 17 mins = 2897 mins

	// 15 = 1 second
	// 15 * 60 = 900 = 1 minute
	// 900 * 60 = 54000 = 1 hour
	// 54000 * 24 = 1296000 = 1 day

	// Calculate each sequence index from the current time

	uint8 hour = 0;
	uint8 minute = 0;
	State::getHourMinutes(time, &hour, &minute);
	uint32 index_date = 18 * time / 1296000;
	if (hour == 23)
		index_date += 18 * minute / 60;

	// Set sequences frames
	_frameMinutes->setFrame(minute);
	_frameHour->setFrame((5 * hour + minute / 12) % 60);
	_frameSun->setFrame((5 * hour + minute / 12) % 120);
	_frameDate->setFrame((uint16)index_date);

	// Adjust z-order and queue
	_frameMinutes->getInfo()->location = 1;
	_frameHour->getInfo()->location = 1;
	_frameSun->getInfo()->location = 1;
	_frameDate->getInfo()->location = 1;

	getScenes()->addToQueue(_frameMinutes);
	getScenes()->addToQueue(_frameHour);
	getScenes()->addToQueue(_frameSun);
	getScenes()->addToQueue(_frameDate);
}

} // End of namespace LastExpress
