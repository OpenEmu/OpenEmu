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

// Actor management module header file

#ifndef SAGA_ACTOR_H
#define SAGA_ACTOR_H

#include "common/savefile.h"

#include "saga/sprite.h"
#include "saga/itedata.h"
#include "saga/saga.h"
#include "saga/font.h"

namespace Saga {

class HitZone;


//#define ACTOR_DEBUG 1 //only for actor pathfinding debug!

#define ACTOR_BARRIERS_MAX 16

#define ACTOR_MAX_STEPS_COUNT 32

#define ACTOR_DIALOGUE_HEIGHT 100

#define ACTOR_LMULT 4

#define ACTOR_SPEED 72

#define ACTOR_CLIMB_SPEED 8

#define ACTOR_COLLISION_WIDTH       32
#define ACTOR_COLLISION_HEIGHT       8

#define ACTOR_DIRECTIONS_COUNT	4	// for ActorFrameSequence

#define ACTOR_SPEECH_STRING_MAX 16	// speech const
#define ACTOR_SPEECH_ACTORS_MAX 8

#define ACTOR_DRAGON_TURN_MOVES 4
#define ACTOR_DRAGON_INDEX 133

#define ACTOR_NO_ENTRANCE -1

#define ACTOR_EXP_KNOCK_RIF 24

#define PATH_NODE_EMPTY -1

#define ACTOR_INHM_SIZE 228

enum ActorDirections {
	kDirectionRight = 0,
	kDirectionLeft = 1,
	kDirectionUp = 2,
	kDirectionDown = 3
};

enum ActorActions {
	kActionWait = 0,
	kActionWalkToPoint = 1,
	kActionWalkToLink = 2,
	kActionWalkDir = 3,
	kActionSpeak = 4,
	kActionAccept = 5,
	kActionStoop = 6,
	kActionLook = 7,
	kActionCycleFrames = 8,
	kActionPongFrames = 9,
	kActionFreeze = 10,
	kActionFall = 11,
	kActionClimb = 12
};

enum ActorFrameIds {
//ITE
	kFrameITEStand = 0,
	kFrameITEWalk = 1,
	kFrameITESpeak = 2,
	kFrameITEGive = 3,
	kFrameITEGesture = 4,
	kFrameITEWait = 5,
	kFrameITEPickUp = 6,
	kFrameITELook = 7,
//IHNM
	kFrameIHNMStand = 0,
	kFrameIHNMSpeak = 1,
	kFrameIHNMWait = 2,
	kFrameIHNMGesture = 3,
	kFrameIHNMWalk = 4
};

enum SpeechFlags {
	kSpeakNoAnimate = 1,
	kSpeakAsync = 2,
	kSpeakSlow = 4,
	kSpeakForceText = 8
};

enum ActorFrameTypes {
	kFrameStand,
	kFrameWalk,
	kFrameSpeak,
	kFrameGive,
	kFrameGesture,
	kFrameWait,
	kFramePickUp,
	kFrameLook
};

// Lookup table to convert 8 cardinal directions to 4
static const int actorDirectionsLUT[8] = {
	kDirectionUp,       // kDirUp
	kDirectionRight,    // kDirUpRight
	kDirectionRight,    // kDirRight
	kDirectionRight,    // kDirDownRight
	kDirectionDown,     // kDirDown
	kDirectionLeft,     // kDirDownLeft
	kDirectionLeft,     // kDirLeft
	kDirectionLeft      // kDirUpLeft
};

enum ActorFlagsEx {
	kActorNoCollide = (1 << 0),
	kActorNoFollow = (1 << 1),
	kActorCollided = (1 << 2),
	kActorBackwards = (1 << 3),
	kActorContinuous = (1 << 4),
	kActorFinalFace = (1 << 5),
	kActorFinishLeft = ((1 << 5) | (kDirLeft << 6)),
	kActorFinishRight = ((1 << 5) | (kDirRight << 6)),
	kActorFinishUp = ((1 << 5) | (kDirUp << 6)),
	kActorFinishDown = ((1 << 5) | (kDirDown << 6)),
	kActorFacingMask = (0xf << 5),
	kActorRandom = (1 << 10)
};

enum PathCellType {
	kPathCellEmpty = -1,
	//kDirUp = 0 .... kDirUpLeft = 7
	kPathCellBarrier = 0x57
};

enum DragonMoveTypes {
	kDragonMoveUpLeft			=	0,
	kDragonMoveUpRight			=	1,
	kDragonMoveDownLeft			=	2,
	kDragonMoveDownRight		=	3,
	kDragonMoveUpLeft_Left		=	4,
	kDragonMoveUpLeft_Right		=	5,
	kDragonMoveUpRight_Left		=	6,
	kDragonMoveUpRight_Right	=	7,
	kDragonMoveDownLeft_Left	=	8,
	kDragonMoveDownLeft_Right	=	9,
	kDragonMoveDownRight_Left	=	10,
	kDragonMoveDownRight_Right	=	11,
	kDragonMoveInvalid			=	12
};

struct PathDirectionData {
	int8 direction;
	int16 x;
	int16 y;
};

struct ActorFrameRange {
	int frameIndex;
	int frameCount;
};

struct ActorFrameSequence {
	ActorFrameRange directions[ACTOR_DIRECTIONS_COUNT];
};

typedef Common::Array<ActorFrameSequence> ActorFrameSequences;

uint pathLine(PointList &pointList, uint idx, const Point &point1, const Point &point2);

struct Location {
	int32 x;					// logical coordinates
	int32 y;					//
	int32 z;					//
	Location() {
		x = y = z = 0;
	}
	void saveState(Common::OutSaveFile *out) {
		out->writeSint32LE(x);
		out->writeSint32LE(y);
		out->writeSint32LE(z);
	}
	void loadState(Common::InSaveFile *in) {
		x = in->readSint32LE();
		y = in->readSint32LE();
		z = in->readSint32LE();
	}

	int distance(const Location &location) const {
		return MAX(ABS(x - location.x), ABS(y - location.y));
	}
	int32 &u() {
		return x;
	}
	int32 &v() {
		return y;
	}
	int32 u() const {
		return x;
	}
	int32 v() const {
		return y;
	}
	int32 uv() const {
		return u() + v();
	}
	void delta(const Location &location, Location &result) const {
		result.x = x - location.x;
		result.y = y - location.y;
		result.z = z - location.z;
	}
	void addXY(const Location &location) {
		x += location.x;
		y += location.y;
	}
	void add(const Location &location) {
		x += location.x;
		y += location.y;
		z += location.z;
	}
	void fromScreenPoint(const Point &screenPoint) {
		x = (screenPoint.x * ACTOR_LMULT);
		y = (screenPoint.y * ACTOR_LMULT);
		z = 0;
	}
	void toScreenPointXY(Point &screenPoint) const {
		screenPoint.x = x / ACTOR_LMULT;
		screenPoint.y = y / ACTOR_LMULT;
	}
	void toScreenPointUV(Point &screenPoint) const {
		screenPoint.x = u();
		screenPoint.y = v();
	}
	void toScreenPointXYZ(Point &screenPoint) const {
		screenPoint.x = x / ACTOR_LMULT;
		screenPoint.y = y / ACTOR_LMULT - z;
	}
	void fromStream(Common::ReadStream &stream) {
		x = stream.readUint16LE();
		y = stream.readUint16LE();
		z = stream.readUint16LE();
	}

#if 0
	// Obsolete function, throws warnings in older versions of GCC
	// (warning: int format, int32 arg)
	// Keeping it around for debug purposes
	void debugPrint(int debuglevel = 0, const char *loc = "Loc:") const {
		debug(debuglevel, "%s %d, %d, %d", loc, x, y, z);
	}
#endif

};

class CommonObjectData {
public:
//constant
	int32 _index;					// index in local array
	uint16 _id;						// object id
	int32 _scriptEntrypointNumber;	// script entrypoint number

//variables
	uint16 _flags;				// initial flags
	int32 _nameIndex;			// index in name string list
	int32 _sceneNumber;			// scene
	int32 _spriteListResourceId;	// sprite list resource id

	Location _location;			// logical coordinates
	Point _screenPosition;		// screen coordinates
	int32 _screenDepth;			//
	int32 _screenScale;			//

	void saveState(Common::OutSaveFile *out) {
		out->writeUint16LE(_flags);
		out->writeSint32LE(_nameIndex);
		out->writeSint32LE(_sceneNumber);
		out->writeSint32LE(_spriteListResourceId);
		_location.saveState(out);
		out->writeSint16LE(_screenPosition.x);
		out->writeSint16LE(_screenPosition.y);
		out->writeSint32LE(_screenDepth);
		out->writeSint32LE(_screenScale);
	}
	void loadState(Common::InSaveFile *in) {
		_flags = in->readUint16LE();
		_nameIndex = in->readSint32LE();
		_sceneNumber = in->readSint32LE();
		_spriteListResourceId = in->readSint32LE();
		_location.loadState(in);
		_screenPosition.x = in->readSint16LE();
		_screenPosition.y = in->readSint16LE();
		_screenDepth = in->readSint32LE();
		_screenScale = in->readSint32LE();
	}

	CommonObjectData() {
		_index = 0;
		_id = 0;
		_scriptEntrypointNumber = 0;

		_flags = 0;
		_nameIndex = 0;
		_sceneNumber = 0;
		_spriteListResourceId = 0;

		_screenDepth = 0;
		_screenScale = 0;
	}
};

typedef CommonObjectData *CommonObjectDataPointer;

typedef Common::List<CommonObjectDataPointer> CommonObjectOrderList;

class ObjectData: public CommonObjectData {
public:
	//constant
	uint16 _interactBits;

	ObjectData() {
		_interactBits = 0;
	}
};

typedef Common::Array<ObjectData> ObjectDataArray;

class ActorData: public CommonObjectData {
public:
	//constant
	SpriteList _spriteList;		// sprite list data

	ActorFrameSequences *_frames;	// Actor's frames
	ActorFrameSequences _framesContainer;	// Actor's frames
	int _frameListResourceId;	// Actor's frame list resource id

	byte _speechColor;			// Actor dialogue color
	//
	bool _inScene;

	//variables
	uint16 _actorFlags;			// dynamic flags
	int32 _currentAction;			// ActorActions type
	int32 _facingDirection;		// orientation
	int32 _actionDirection;
	int32 _actionCycle;
	uint16 _targetObject;
	const HitZone *_lastZone;

	int32 _cycleFrameSequence;
	uint8 _cycleDelay;
	uint8 _cycleTimeCount;
	uint8 _cycleFlags;

	int16 _fallVelocity;
	int16 _fallAcceleration;
	int16 _fallPosition;

	uint8 _dragonBaseFrame;
	uint8 _dragonStepCycle;
	uint8 _dragonMoveType;

	int32 _frameNumber;			// current frame number

	ByteArray _tileDirections;

	Common::Array<Point> _walkStepsPoints;

	int32 _walkStepsCount;
	int32 _walkStepIndex;

	Location _finalTarget;
	Location _partialTarget;
	int32 _walkFrameSequence;

public:
	ActorData();

	void saveState(Common::OutSaveFile *out);
	void loadState(uint32 version, Common::InSaveFile *in);

	void cycleWrap(int cycleLimit);
	void addWalkStepPoint(const Point &point);
	bool shareFrames() {
		return ((_frames != NULL) && (_frames != &_framesContainer));
	}
};

typedef Common::Array<ActorData> ActorDataArray;

struct ProtagStateData {
	ActorFrameSequences _frames;	// Actor's frames
};


struct SpeechData {
	int speechColor[ACTOR_SPEECH_ACTORS_MAX];
	int outlineColor[ACTOR_SPEECH_ACTORS_MAX];
	int speechFlags;
	const char *strings[ACTOR_SPEECH_STRING_MAX];
	Rect speechBox;
	Rect drawRect;
	int stringsCount;
	int slowModeCharIndex;
	uint16 actorIds[ACTOR_SPEECH_ACTORS_MAX];
	int actorsCount;
	int sampleResourceId;
	bool playing;
	int playingTime;

	SpeechData() {
		memset(this, 0, sizeof(*this));
	}

	FontEffectFlags getFontFlags(int i) {
		if (outlineColor[i] != 0) {
			return kFontOutline;
		} else {
			return kFontNormal;
		}
	}
};

typedef int (*CompareFunction) (const CommonObjectDataPointer& a, const CommonObjectDataPointer& b);

class Actor {
	friend class IsoMap;
	friend class SagaEngine;
	friend class Puzzle;
public:

	Actor(SagaEngine *vm);
	~Actor();

	void cmdActorWalkTo(int argc, const char **argv);

	bool validActorId(uint16 id) {
		return (id == ID_PROTAG) || ((id >= objectIndexToId(kGameObjectActor, 0)) && (id < objectIndexToId(kGameObjectActor, _actors.size())));
	}
	int actorIdToIndex(uint16 id) { return (id == ID_PROTAG) ? 0 : objectIdToIndex(id); }
	uint16 actorIndexToId(int index) { return (index == 0) ? ID_PROTAG : objectIndexToId(kGameObjectActor, index); }
	ActorData *getActor(uint16 actorId);
	ActorData *getFirstActor() { return &_actors.front(); }

// clarification: Obj - means game object, such Hat, Spoon etc,  Object - means Actor,Obj,HitZone,StepZone

	bool validObjId(uint16 id) { return (id >= objectIndexToId(kGameObjectObject, 0)) && (id < objectIndexToId(kGameObjectObject, _objs.size())); }
	int objIdToIndex(uint16 id) { return objectIdToIndex(id); }
	uint16 objIndexToId(int index) { return objectIndexToId(kGameObjectObject, index); }
	ObjectData *getObj(uint16 objId);

	int getObjectScriptEntrypointNumber(uint16 id) {
		int objectType;
		objectType = objectTypeId(id);
		if (!(objectType & (kGameObjectObject | kGameObjectActor))) {
			error("Actor::getObjectScriptEntrypointNumber wrong id 0x%X", id);
		}
		return (objectType == kGameObjectObject) ? getObj(id)->_scriptEntrypointNumber : getActor(id)->_scriptEntrypointNumber;
	}
	int getObjectFlags(uint16 id) {
		int objectType;
		objectType = objectTypeId(id);
		if (!(objectType & (kGameObjectObject | kGameObjectActor))) {
			error("Actor::getObjectFlags wrong id 0x%X", id);
		}
		return (objectType == kGameObjectObject) ? getObj(id)->_flags : getActor(id)->_flags;
	}

	void direct(int msec);
	void drawActors();
	void updateActorsScene(int actorsEntrance);			// calls from scene loading to update Actors info

	void drawSpeech();

#ifdef ACTOR_DEBUG
	void drawPathTest();
#endif

	uint16 hitTest(const Point &testPoint, bool skipProtagonist);
	void takeExit(uint16 actorId, const HitZone *hitZone);
	bool actorEndWalk(uint16 actorId, bool recurse);
	bool actorWalkTo(uint16 actorId, const Location &toLocation);
	int getFrameType(ActorFrameTypes frameType);
	ActorFrameRange *getActorFrameRange(uint16 actorId, int frameType);
	void actorFaceTowardsPoint(uint16 actorId, const Location &toLocation);
	void actorFaceTowardsObject(uint16 actorId, uint16 objectId);

	void realLocation(Location &location, uint16 objectId, uint16 walkFlags);

//	speech
	void actorSpeech(uint16 actorId, const char **strings, int stringsCount, int sampleResourceId, int speechFlags);
	void nonActorSpeech(const Common::Rect &box, const char **strings, int stringsCount, int sampleResourceId, int speechFlags);
	void simulSpeech(const char *string, uint16 *actorIds, int actorIdsCount, int speechFlags, int sampleResourceId);
	void setSpeechColor(int speechColor, int outlineColor) {
		_activeSpeech.speechColor[0] = speechColor;
		_activeSpeech.outlineColor[0] = outlineColor;
	}
	void abortAllSpeeches();
	void abortSpeech();
	bool isSpeaking() {
		return _activeSpeech.stringsCount > 0;
	}

	int isForcedTextShown() {
		return _activeSpeech.speechFlags & kSpeakForceText;
	}

	void saveState(Common::OutSaveFile *out);
	void loadState(Common::InSaveFile *in);

	void setProtagState(int state);
	int getProtagState() { return _protagState; }

	void loadActorList(int protagonistIdx, int actorCount, int actorsResourceID,
				  int protagStatesCount, int protagStatesResourceID);
	void loadObjList(int objectCount, int objectsResourceID);

protected:
	friend class Script;
	void loadActorResources(ActorData *actor);
	void loadFrameList(int frameListResourceId, ActorFrameSequences &frames);
private:
	void stepZoneAction(ActorData *actor, const HitZone *hitZone, bool exit, bool stopped);
	void loadActorSpriteList(ActorData *actor);

	void drawOrderListAdd(const CommonObjectDataPointer& element, CompareFunction compareFunction);
	void createDrawOrderList();
	bool calcScreenPosition(CommonObjectData *commonObjectData);
	bool getSpriteParams(CommonObjectData *commonObjectData, int &frameNumber, SpriteList *&spriteList);

	bool followProtagonist(ActorData *actor);
	void findActorPath(ActorData *actor, const Point &fromPoint, const Point &toPoint);
	void handleSpeech(int msec);
	void handleActions(int msec, bool setup);
	bool validPathCellPoint(const Point &testPoint) {
		return !((testPoint.x < 0) || (testPoint.x >= _xCellCount) ||
			(testPoint.y < 0) || (testPoint.y >= _yCellCount));
	}
	void setPathCell(const Point &testPoint, int8 value) {
#ifdef ACTOR_DEBUG
		if (!validPathCellPoint(testPoint)) {
			error("Actor::setPathCell wrong point");
		}
#endif
		_pathCell[testPoint.x + testPoint.y * _xCellCount] = value;
	}
	int8 getPathCell(const Point &testPoint) {
#ifdef ACTOR_DEBUG
		if (!validPathCellPoint(testPoint)) {
			error("Actor::getPathCell wrong point");
		}
#endif
		return _pathCell[testPoint.x + testPoint.y * _xCellCount];
	}
	bool scanPathLine(const Point &point1, const Point &point2);
	int fillPathArray(const Point &fromPoint, const Point &toPoint, Point &bestPoint);
	void setActorPath(ActorData *actor, const Point &fromPoint, const Point &toPoint);
	void pathToNode();
	void condenseNodeList();
	void removeNodes();
	void nodeToPath();
	void removePathPoints();
	bool validFollowerLocation(const Location &location);
	void moveDragon(ActorData *actor);


protected:
//constants
	ActorDataArray _actors;

	ObjectDataArray _objs;

	SagaEngine *_vm;
	ResourceContext *_actorContext;

	int _lastTickMsec;
	CommonObjectOrderList _drawOrderList;

//variables
public:
	ActorData *_centerActor;
	ActorData *_protagonist;
	int _handleActionDiv;

	Rect _speechBoxScript;

	StringsTable _objectsStrings;
	StringsTable _actorsStrings;

protected:
	SpeechData _activeSpeech;
	int _protagState;
	bool _dragonHunt;

private:
	Common::Array<ProtagStateData> _protagStates;

//path stuff
	struct PathNode {
		Point point;
		int link;

		PathNode() : link(0) {}
		PathNode(const Point &p) : point(p), link(0) {}
		PathNode(const Point &p, int l) : point(p), link(l) {}
	};
	typedef Common::Array<PathNode> PathNodeList;

	Rect _barrierList[ACTOR_BARRIERS_MAX];
	int _barrierCount;
	Common::Array<int8> _pathCell;

	int _xCellCount;
	int _yCellCount;
	Rect _pathRect;

	PointList _pathList;
	uint _pathListIndex;

	PathNodeList _pathNodeList;

public:
#ifdef ACTOR_DEBUG
#ifndef SAGA_DEBUG
	#error You must also define SAGA_DEBUG
#endif
//path debug - use with care
	struct DebugPoint {
		Point point;
		byte color;

		DebugPoint() : color(0) {}

		DebugPoint(const Point &p, byte c): point(p), color(c) {}
	};

	Common::Array<DebugPoint> _debugPoints;
	uint _debugPointsCount;
	// we still need this trick to speedup debug points addition
	void addDebugPoint(const Point &point, byte color) {
		if (_debugPointsCount < _debugPoints.size()) {
			_debugPoints[_debugPointsCount].point = point;
			_debugPoints[_debugPointsCount].color = color;
		} else {
			_debugPoints.push_back(DebugPoint(point, color));
		}
		++_debugPointsCount;
	}
#endif
};

} // End of namespace Saga

#endif
