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

#include "lastexpress/menu/trainline.h"

#include "lastexpress/data/sequence.h"

#include "lastexpress/game/scenes.h"

#include "lastexpress/helpers.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"

namespace LastExpress {

// Information about the cities on the train line
static const struct {
	uint8 frame;
	TimeValue time;
} _trainCities[31] = {
	{0, kTimeCityParis},
	{9, kTimeCityEpernay},
	{11, kTimeCityChalons},
	{16, kTimeCityBarLeDuc},
	{21, kTimeCityNancy},
	{25, kTimeCityLuneville},
	{35, kTimeCityAvricourt},
	{37, kTimeCityDeutschAvricourt},
	{40, kTimeCityStrasbourg},
	{53, kTimeCityBadenOos},
	{56, kTimeCityKarlsruhe},
	{60, kTimeCityStuttgart},
	{63, kTimeCityGeislingen},
	{66, kTimeCityUlm},
	{68, kTimeCityAugsburg},
	{73, kTimeCityMunich},
	{84, kTimeCitySalzbourg},
	{89, kTimeCityAttnangPuchheim},
	{97, kTimeCityWels},
	{100, kTimeCityLinz},
	{104, kTimeCityAmstetten},
	{111, kTimeCityVienna},
	{120, kTimeCityPoszony},
	{124, kTimeCityGalanta},
	{132, kTimeCityBudapest},
	{148, kTimeCityBelgrade},
	/* Line 1 ends at 150 - line 2 begins at 0 */
	{157, kTimeCityNish},
	{165, kTimeCityTzaribrod},
	{174, kTimeCitySofia},
	{198, kTimeCityAdrianople},
	{210, kTimeCityConstantinople}
};

TrainLine::TrainLine(LastExpressEngine *engine) : _engine(engine), _frameLine1(NULL), _frameLine2(NULL) {
	_frameLine1 = new SequenceFrame(loadSequence("line1.seq"), 0, true);
	_frameLine2 = new SequenceFrame(loadSequence("line2.seq"), 0, true);
}

TrainLine::~TrainLine() {
	SAFE_DELETE(_frameLine1);
	SAFE_DELETE(_frameLine2);

	// Zero passed pointers
	_engine = NULL;
}

void TrainLine::clear() {
	getScenes()->removeFromQueue(_frameLine1);
	getScenes()->removeFromQueue(_frameLine2);
}

// Draw the train line at the time
//  line1: 150 frames (=> Belgrade)
//  line2: 61 frames (=> Constantinople)
void TrainLine::draw(uint32 time) {
	assert(time >= kTimeCityParis && time <= kTimeCityConstantinople);

	// Check that sequences have been loaded
	if (!_frameLine1 || !_frameLine2)
		error("[TrainLine::draw] Line sequences have not been loaded correctly");

	// Clear existing frames
	clear();

	// Get the index of the last city the train has visited
	uint index = 0;
	for (uint i = 0; i < ARRAYSIZE(_trainCities); i++)
		if ((uint32)_trainCities[i].time <= time)
			index = i;

	uint16 frame;
	if (time > (uint32)_trainCities[index].time) {
		// Interpolate linearly to use a frame between the cities
		uint8 diffFrames = _trainCities[index + 1].frame - _trainCities[index].frame;
		uint diffTimeCities = (uint)(_trainCities[index + 1].time - _trainCities[index].time);
		uint traveledTime = (time - (uint)_trainCities[index].time);
		frame = (uint16)(_trainCities[index].frame + (traveledTime * diffFrames) / diffTimeCities);
	} else {
		// Exactly on the city
		frame = _trainCities[index].frame;
	}

	// Set frame, z-order and queue
	if (frame < 150) {
		_frameLine1->setFrame(frame);

		_frameLine1->getInfo()->location = 1;
		getScenes()->addToQueue(_frameLine1);
	} else {
		// We passed Belgrade
		_frameLine1->setFrame(149);
		_frameLine2->setFrame(frame - 150);

		_frameLine1->getInfo()->location = 1;
		_frameLine2->getInfo()->location = 1;

		getScenes()->addToQueue(_frameLine1);
		getScenes()->addToQueue(_frameLine2);
	}
}

} // End of namespace LastExpress
