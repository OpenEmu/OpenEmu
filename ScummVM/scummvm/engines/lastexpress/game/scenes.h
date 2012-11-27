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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef LASTEXPRESS_SCENEMANAGER_H
#define LASTEXPRESS_SCENEMANAGER_H

#include "lastexpress/data/scene.h"

#include "common/hashmap.h"
#include "common/list.h"

namespace LastExpress {

class LastExpressEngine;
class SceneLoader;
class Sequence;
class SequenceFrame;

class SceneManager {
public:
	enum CheckPositionType {
		kCheckPositionLookingUp,
		kCheckPositionLookingDown,
		kCheckPositionLookingAtDoors,
		kCheckPositionLookingAtClock
	};

	SceneManager(LastExpressEngine *engine);
	~SceneManager();

	// Scene cache
	void loadSceneDataFile(ArchiveIndex archive);
	Scene *get(SceneIndex sceneIndex) { return _sceneLoader->get(sceneIndex); }

	// Scene loading
	void setScene(SceneIndex sceneIndex);
	void loadScene(SceneIndex sceneIndex);
	void loadSceneFromObject(ObjectIndex object, bool alternate = false);
	void loadSceneFromItem(InventoryItem item);
	void loadSceneFromItemPosition(InventoryItem item);
	void loadSceneFromPosition(CarIndex car, Position position, int param3 = -1);

	// Scene drawing & processing
	void drawScene(SceneIndex sceneIndex);
	void processScene();
	SceneIndex processIndex(SceneIndex sceneIndex);

	// Checks
	bool checkPosition(SceneIndex sceneIndex, CheckPositionType type) const;
	bool checkCurrentPosition(bool doCheckOtherCars) const;

	// Train
	void updateDoorsAndClock();
	void resetDoorsAndClock();

	// Sequence queue
	void drawFrames(bool refreshScreen);
	void addToQueue(SequenceFrame * const frame);
	void removeFromQueue(SequenceFrame *frame);
	void removeAndRedraw(SequenceFrame **frame, bool doRedraw);
	void resetQueue();
	void setCoordinates(SequenceFrame *frame);
	void setCoordinates(const Common::Rect &rect);

	// Helpers
	SceneIndex getSceneIndexFromPosition(CarIndex car, Position position, int param3 = -1);

	void setFlagDrawSequences() { _flagDrawSequences = true; }

private:
	LastExpressEngine *_engine;
	SceneLoader       *_sceneLoader;     ///< Scene loader

	// Flags
	bool _flagNoEntity;
	bool _flagDrawEntities;
	bool _flagDrawSequences;
	bool _flagCoordinates;

	Common::Rect _coords;

	// Train sequences
	Common::List<SequenceFrame *> _doors;
	SequenceFrame *_clockHours;
	SequenceFrame *_clockMinutes;

	// Sequence queue
	Common::List<SequenceFrame *> _queue;

	// Scene processing
	void preProcessScene(SceneIndex *index);
	void postProcessScene();

	void resetCoordinates();
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_SCENEMANAGER_H
