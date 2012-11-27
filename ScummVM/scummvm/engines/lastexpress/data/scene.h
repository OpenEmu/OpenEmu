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

#ifndef LASTEXPRESS_SCENE_H
#define LASTEXPRESS_SCENE_H

/*
	Scene format (CDTRAIN.DAT)

	(text:00484750)
	header (24 bytes)
	    char {8}    - entry name (null terminated)
	    byte {1}    - 0xCD
	    uint16 {2}  - number of scenes (for first entry - always 0 after?)
	    uint16 {2}  - 11 ??
	    uint16 {2}  - car
	    byte {1}    - camera position (used to get the proper sequences to show)
	    byte {1}    - type
	    byte {1}    - param1
	    byte {1}    - param2
	    byte {1}    - param3
	    uint32 {4}  - Offset to hotspot info struct

	    probably contains cursor type too / scene index : 0 - 2500 (max)

	hotspot info (24 bytes)
	    uint16 {2}  - left
	    uint16 {2}  - right
	    uint16 {2}  - top
	    uint16 {2}  - bottom
	    uint32 {4}  - scene coords data
	    uint16 {2}  - scene
	    byte {1}    - location;
	    byte {1}    - action;
	    byte {1}    - param1;
	    byte {1}    - param2;
	    byte {1}    - param3
	    byte {1}    - cursor
	    uint32{4}   - offset to next hotpost

	coords data (9 bytes)
	    uint32 {4}    - ??
	    uint32 {4}    - ??
	    byte {1}       - ??
	   uint32 {4}     - offset to next coords data structure

*/

#include "lastexpress/drawable.h"
#include "lastexpress/shared.h"

#include "common/array.h"
#include "common/rect.h"

namespace Common {
class SeekableReadStream;
class String;
}

namespace LastExpress {

class Scene;

class SceneHotspot {
public:
	enum Action {
		kActionInventory = 1,
		kActionSavePoint = 2,
		kActionPlaySound = 3,
		kActionPlayMusic = 4,
		kActionKnockOnDoor = 5,
		kActionCompartment = 6,
		kActionPlaySounds = 7,
		kActionPlayAnimation = 8,
		kActionOpenCloseObject = 9,
		kActionObjectUpdateLocation2 = 10,
		kActionSetItemLocation = 11,
		kAction12 = 12,
		kActionPickItem = 13,
		kActionDropItem = 14,
		kAction15 = 15,
		kActionEnterCompartment = 16,
		kActionGetOutsideTrain = 18,
		kActionSlip = 19,
		kActionGetInsideTrain = 20,
		kActionClimbUpTrain = 21,
		kActionClimbDownTrain = 22,
		kActionJumpUpDownTrain = 23,
		kActionUnbound = 24,
		kAction25 = 25,
		kAction26 = 26,
		kAction27 = 27,
		kActionConcertSitCough = 28,
		kAction29 = 29,
		kActionCatchBeetle = 30,
		kActionExitCompartment = 31,
		kAction32 = 32,
		KActionUseWhistle = 33,
		kActionOpenMatchBox = 34,
		kActionOpenBed = 35,
		kActionDialog = 37,
		kActionEggBox = 38,
		kAction39 = 39,
		kActionBed = 40,
		kAction41 = 41,
		kAction42 = 42,
		kActionSwitchChapter = 43,
		kAction44 = 44
	};

	struct SceneCoord {
		int32 field_0;
		int32 field_4;
		byte field_8;
		uint32 next;

		SceneCoord() {
			field_0 = 0;
			field_4 = 0;
			field_8 = 0;
			next = 0;
		}
	};

	Common::Rect rect;
	uint32 coordsOffset;
	SceneIndex scene;
	byte location;
	Action action;
	byte param1;
	byte param2;
	byte param3;
	byte cursor;
	uint32 next;

	SceneHotspot() {}
	~SceneHotspot();
	static SceneHotspot *load(Common::SeekableReadStream *stream);

	bool isInside(const Common::Point &point);

	Common::String toString() const;

private:
	Common::Array<SceneCoord *> _coords;
};

class SceneLoader {
public:
	SceneLoader();
	~SceneLoader();

	bool load(Common::SeekableReadStream *stream);
	Scene *get(SceneIndex index);

	uint32 count() const { return _scenes.size() - 1; }

private:
	Common::SeekableReadStream *_stream;
	Common::Array<Scene *> _scenes;

	void clear();
};

class Scene : public Drawable {
public:
	// Enumerations
	enum Type {
		// PreProcess
		kTypeObject = 1,
		kTypeItem = 2,
		kTypeItem2 = 3,
		kTypeObjectItem = 4,
		kTypeItem3 = 5,
		kTypeObjectLocation2 = 6,
		kTypeCompartments = 7,
		kTypeCompartmentsItem = 8,

		// PostProcess
		kTypeList = 128,
		kTypeSavePointChapter = 129,
		kTypeLoadBeetleSequences = 130,
		kTypeGameOver = 131,
		kTypeReadText = 132,
		kType133 = 133
	};

	// Data
	EntityPosition entityPosition;
	Location location;
	CarIndex car;
	Position position;
	Type type;
	byte param1;
	byte param2;
	byte param3;

	~Scene();

	Common::Rect draw(Graphics::Surface *surface);

	// Hotspots
	Common::Array<SceneHotspot *> *getHotspots() { return &_hotspots; }
	bool checkHotSpot(const Common::Point &coord, SceneHotspot **hotspot);
	SceneHotspot *getHotspot(uint index = 0);

	Common::String toString();

private:
	char _name[8];
	byte _sig;
	uint32 _hotspot;

	Scene() {}
	Common::Array<SceneHotspot *> _hotspots;

	static Scene *load(Common::SeekableReadStream *stream);
	void loadHotspots(Common::SeekableReadStream *stream);

	void clear();

	// Only allow full access for loading the scene and the hotspots
	friend bool SceneLoader::load(Common::SeekableReadStream *stream);
	friend Scene *SceneLoader::get(SceneIndex index);
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_SCENE_H
