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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TSAGE_RINGWORLD_SCENES1_H
#define TSAGE_RINGWORLD_SCENES1_H

#include "common/scummsys.h"
#include "tsage/ringworld/ringworld_logic.h"
#include "tsage/ringworld/ringworld_speakers.h"
#include "tsage/converse.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/sound.h"

namespace TsAGE {

namespace Ringworld {

using namespace TsAGE;

class Scene10 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		virtual void signal();
	};
	class Action2 : public Action {
	public:
		virtual void signal();
	};
public:
	Speaker _speakerSText;
	Speaker _speakerQText;
	Action1 _action1;
	Action2 _action2;
	SceneObject _object1, _object2, _object3;
	SceneObject _object4, _object5, _object6;

	virtual void stripCallback(int v);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
};

class Scene15 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		virtual void signal();
		virtual void dispatch();
	};
public:
	Action1 _action1;
	SceneObject _object1;
	ASound _soundHandler;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
};

class Scene20 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		virtual void signal();
	};
	class Action2 : public Action {
	public:
		virtual void signal();
	};
	class Action3 : public Action {
	public:
		virtual void signal();
	};
	class Action4 : public Action {
	public:
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager;
	SpeakerQText _speakerQText;
	SpeakerGameText _speakerGameText;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	SceneObject _sceneObject1, _SceneObjectExt, _sceneObject3, _sceneObject4, _sceneObject5;
	ASound _sound;
public:
	Scene20();
	virtual ~Scene20() {}

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene30 : public Scene {
	/* Scene objects */
	// Doorway beam sensor
	class BeamObject : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	// Doorway object
	class DoorObject : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	// Kzin object
	class KzinObject : public SceneObject {
	public:
		virtual void doAction(int action);
	};

	/* Actions */
	class BeamAction : public Action {
	public:
		virtual void signal();
	};
	class KzinAction : public Action {
	public:
		virtual void signal();
	};
	class RingAction : public Action {
	public:
		virtual void signal();
	};
	class TalkAction : public Action {
	public:
		virtual void signal();
	};

public:
	ASound _sound;
	DisplayHotspot _groundHotspot, _wallsHotspot, _courtyardHotspot, _treeHotspot;
	BeamObject _beam;
	DoorObject _door;
	KzinObject _kzin;

	BeamAction _beamAction;
	KzinAction _kzinAction;
	RingAction _ringAction;
	TalkAction _talkAction;
	SequenceManager _sequenceManager;

	SpeakerSR _speakerSR;
	SpeakerQL _speakerQL;
	SpeakerSText _speakerSText;
	SpeakerQText _speakerQText;
public:
	Scene30();
	virtual ~Scene30() {}

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene40 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		virtual void signal();
	};
	class Action2 : public Action {
	public:
		virtual void signal();
	};
	class Action3 : public Action {
	public:
		virtual void signal();
	};
	class Action4 : public Action {
	public:
		virtual void signal();
	};
	class Action5 : public Action {
	public:
		virtual void signal();
	};
	class Action6 : public Action {
	public:
		virtual void signal();
	};
	class Action7 : public Action {
	public:
		virtual void signal();
	};
	class Action8 : public Action {
	public:
		virtual void signal();
		virtual void dispatch();
	};

	/* Objects */
	class DyingKzin : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Assassin : public SceneObject {
	public:
		virtual void doAction(int action);
	};

	/* Items */
	class Item2 : public SceneHotspot {
	public:
		virtual void doAction(int action);
	};
	class Item6 : public SceneHotspot {
	public:
		virtual void doAction(int action);
	};
	class Item8 : public SceneHotspot {
	public:
		virtual void doAction(int action);
	};
public:
	SequenceManager _sequenceManager;
	SpeakerSL _speakerSL;
	SpeakerQR _speakerQR;
	SpeakerQText _speakerQText;
	SpeakerSText _speakerSText;
	SpeakerGameText _speakerGameText;
	ASound _soundHandler;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	Action7 _action7;
	Action8 _action8;
	SceneObject _object1, _object2, _object3;
	DyingKzin _dyingKzin;
	Assassin _assassin;
	SceneObject _doorway, _object7, _object8;
	DisplayHotspot _item1;
	Item2 _item2;
	DisplayHotspot _item3, _item4, _item5;
	Item6 _item6;
	DisplayHotspot _item7, _item8;

	Scene40();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene50 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		virtual void signal();
	};
	class Action2 : public Action {
	public:
		virtual void signal();
	};
	class Action3 : public Action {
	public:
		virtual void signal();
	};

	/* Objects */
	class Object1 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Object2 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Object3 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Object4 : public SceneObject {
	public:
		virtual void doAction(int action);
	};

public:
	SequenceManager _sequenceManager;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Object1 _object1;
	Object2 _object2;
	Object3 _object3;
	Object4 _object4;
	Rect _doorwayRect;
	SpeakerSText _speakerSText;
	SpeakerQText _speakerQText;
	DisplayHotspot _item0, _item1, _item2;
	DisplayHotspot _item3, _item4, _item5;

	Scene50();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene60 : public Scene {
	class Action1 : public Action {
	public:
		virtual void signal();
	};
	class Action2 : public Action {
	public:
		virtual void signal();
	};
	class PrevObject : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class NextObject : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class ExitObject : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class MessageObject : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class ControlObject : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class SlaveObject : public SceneObjectExt {
	public:
		virtual void doAction(int action);
	};
	class MasterObject : public SceneObjectExt {
	public:
		virtual void doAction(int action);
	};
	class FloppyDrive : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Item1 : public SceneHotspot {
	public:
		virtual void doAction(int action);
	};
	class Item : public SceneHotspot {
	public:
		int _messageNum, _sceneMode;

		Item(int sceneRegionId, int messageNum, int sceneMode) {
			_sceneRegionId = sceneRegionId;
			_messageNum = messageNum;
			_sceneMode = sceneMode;
		}
		virtual void doAction(int action);
	};

public:
	GfxButton _gfxButton;
	SequenceManager _sequenceManager;
	SpeakerQText _speakerQText;
	SpeakerSText _speakerSText;
	Action1 _action1;
	Action2 _action2;
	SceneObject _rose;
	PrevObject _prevButton;
	NextObject _nextButton;
	ExitObject _exitButton;
	MessageObject _message;
	ControlObject _controlButton;
	SlaveObject _slaveButton;
	MasterObject _masterButton;
	FloppyDrive _floppyDrive;
	SceneObject _redLights;
	Item1 _item1;
	Item _item2, _item3, _item4, _item5, _item6;
	ASound _soundHandler1;
	ASound _soundHandler2;
	ASound _soundHandler3;

	Scene60();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
};

class Scene90 : public Scene {
	class Action1 : public Action {
	public:
		virtual void signal();
	};
	class Object1 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Object2 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
public:
	SequenceManager _sequenceManager;
	SpeakerSText _speakerSText;
	SpeakerQText _speakerQText;
	SpeakerQL _speakerQL;
	SpeakerSR _speakerSR;
	SpeakerMText _speakerMText;
	Action1 _action1;
	Object1 _object1;
	Object2 _object2;
	DisplayObject _object3, _object4, _object5;
	SceneObject _object6;
	DisplayHotspot _item1, _item2, _item3;
	ASound _soundHandler1, _soundHandler2;

	Scene90();

	virtual void stripCallback(int v);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene95 : public Scene {
	class Action1 : public ActionExt {
	public:
		virtual void signal();
	};

public:
	Action1 _action1;
	SceneObject _object1, _object2, _object3;
	ASound _soundHandler;

	Scene95();
	virtual void postInit(SceneObjectList *OwnerList);
};

class Scene6100 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		virtual void signal();
	};
	class Action2 : public Action {
	public:
		virtual void signal();
	};
	class Action3 : public Action {
	public:
		virtual void signal();
	};
	class Action4 : public Action {
	public:
		virtual void signal();
	};
	class Action5 : public Action {
	public:
		virtual void dispatch();
	};
	class GetBoxAction : public Action {
	public:
		virtual void signal();
		virtual void dispatch();
	};
	class Action7 : public Action {
	public:
		virtual void signal();
	};

	/* Objects */
	class Object : public SceneObject {
	public:
		FloatSet _floats;

		virtual void synchronize(Serializer &s);
	};
	class ProbeMover : public NpcMover {
	public:
		virtual void dispatch();
	};

	/* Items */
	class Item1 : public SceneItem {
	public:
		virtual void doAction(int action);
	};

public:
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	GetBoxAction _getBoxAction;
	Action7 _action7;
	ASound _soundHandler;
	Speaker _speaker1;
	SpeakerQR _speakerQR;
	SpeakerSL _speakerSL;
	SceneObject _object1, _object2, _object3;
	Object _rocks, _probe;
	Object _sunflower1, _sunflower2, _sunflower3;
	SceneText _sceneText;
	SceneItem _item1;

	int _turnAmount, _angle, _speed, _fadePercent;
	int _hitCount;
	bool _rocksCheck;
	Object *_objList[4];
	bool _msgActive;

	Scene6100();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void process(Event &event);
	virtual void dispatch();
	void showMessage(const Common::String &msg, int color, Action *action);

};

} // End of namespace Ringworld

} // End of namespace TsAGE

#endif
