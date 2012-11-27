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

#ifndef TSAGE_BLUEFORCE_SCENES1_H
#define TSAGE_BLUEFORCE_SCENES1_H

#include "common/scummsys.h"
#include "tsage/blue_force/blueforce_logic.h"
#include "tsage/blue_force/blueforce_speakers.h"
#include "tsage/converse.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/sound.h"

namespace TsAGE {

namespace BlueForce {

using namespace TsAGE;

class Scene100: public SceneExt {
	/* Support classes */
	class Text: public SceneText {
	public:
		virtual Common::String getClassName() { return "BF100Text"; }
		virtual void dispatch();
	};

	/* Actions */
	class Action1: public ActionExt {
	private:
		void setTextStrings(const Common::String &msg1, const Common::String &msg2, Action *action);
	public:
		Text _sceneText1;
		SceneText _sceneText2;
		int _textHeight;

		virtual Common::String getClassName() { return "BF100Action1"; }
		virtual void synchronize(Serializer &s) {
			ActionExt::synchronize(s);
			s.syncAsSint16LE(_textHeight);
		}
		virtual void signal();
	};
	class Action2: public ActionExt {
	public:
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager;
	Action1 _action1;
	Action2 _action2;
	ScenePalette _scenePalette;
	NamedObject _object1, _object2, _object3, _object4, _object5;
	int _index;

	Scene100();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene109: public PalettedScene {
	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};
	class Action3: public Action {
	public:
		virtual void signal();
	};

public:
	SequenceManager _sequenceManager1, _sequenceManager2, _sequenceManager3;
	SequenceManager _sequenceManager4, _sequenceManager5, _sequenceManager6;
	SequenceManager _sequenceManager7, _sequenceManager8;
	SceneObject _object1, _object2, _protaginist2, _protaginist1, _cop1;
	SceneObject _drunk, _cop2, _bartender, _beerSign, _animationInset;
	IntroSceneText _text;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
public:
	Scene109();

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene110: public SceneExt {
	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};
	class Action3: public Action {
	public:
		virtual void signal();
		virtual void dispatch();
	};
	class Action4: public Action {
	public:
		virtual void signal();
		virtual void dispatch();
	};
	class Action5: public Action {
	public:
		virtual void signal();
	};
public:
	NamedObject _object1, _object2, _object3, _object4, _object5, _object6, _object7, _object8, _object9, _object10;
	ASound _sound;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
public:
	virtual void postInit(SceneObjectList *OwnerList = NULL);
};

class Scene114: public SceneExt {
	/* Objects */
	class Vechile: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Door: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	SequenceManager _sequenceManager1;
	Vechile _vechile;
	Door _door;
	NamedObject _lyle;
	NamedHotspot _item1;
public:
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene115: public SceneExt {
	/* Objects */
	class Kate: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Tony: public NamedObject {
	public:
		int _talkToTonyCtr2;
		virtual bool startAction(CursorType action, Event &event);
	};
	class Object3: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Object4: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Custom class */
	class EventHandler1: public EventHandler {
	public:
		virtual Common::String getClassName() { return "Scene115_EventHandler1"; }
		virtual void dispatch();
	};

	/* Items */
	class Jukebox: public NamedHotspot {
		SequenceManager _sequenceManager6;
	public:
		int _jokeboxPlayingCtr;

		Jukebox();
		virtual bool startAction(CursorType action, Event &event);
		virtual void signal();
		virtual void synchronize(Serializer &s);
	};
	class Item10: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item14: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};
	class Action3: public Action {
	public:
		virtual void signal();
	};
	class Action4: public Action {
	public:
		virtual void signal();
	};
	class Action5: public Action {
	public:
		virtual void signal();
	};
	class Action6: public Action {
	public:
		virtual void signal();
	};
	class Action7: public Action {
	public:
		virtual void signal();
	};
	class Action8: public Action {
	public:
		virtual void signal();
	};
	class Action9: public Action {
	public:
		virtual void signal();
	};

	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;
	SequenceManager _sequenceManager3;
	SequenceManager _sequenceManager4;
	SequenceManager _sequenceManager5;
	Kate _kate;
	Tony _tony;
	Object3 _object3;
	Object4 _object4;
	SceneObject _object5,  _object6,  _neonSign,  _object8,  _object9;
	SceneObject _object10, _object11, _object12, _object13;
	Jukebox _itemJukebox;
	EventHandler1 _eventHandler1;
	NamedHotspot _item2, _item3, _item4, _item5, _item6, _item7, _item8, _item9;
	Item10 _item10;
	NamedHotspot _item11, _item12, _item13;
	Item14 _item14;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	Action7 _action7;
	Action8 _action8;
	Action9 _action9;
	SpeakerGameText _gameTextSpeaker;
	SpeakerKate _kateSpeaker;
	SpeakerTony _tonySpeaker;
	SpeakerJakeJacket _jakeJacketSpeaker;
	SpeakerJakeUniform _jakeUniformSpeaker;
	SpeakerLyleHat _lyleHatSpeaker;
	ASound _sound1;
	int _lineNumModifier;
	int _jukeboxPlaying;
	int _talkToTonyCtr;
public:
	Scene115();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
};

class Scene125: public SceneExt {
	class Action1: public Action {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
		virtual void dispatch();
	};
	class Action3: public Action {
	public:
		virtual void signal();
		virtual void dispatch();
	};
	class Action4: public Action {
	public:
		virtual void signal();
		virtual void dispatch();
	};
	class Action5: public Action {
	public:
		virtual void signal();
	};
	class Action6: public Action {
	public:
		virtual void signal();
		virtual void dispatch();
	};

public:
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	NamedObject _object1, _object2, _object3, _object4, _object5;
	NamedObject _object6, _object7, _object8, _object9;
	ASoundExt _soundExt1;
	ASoundExt _soundExt2;

	void postInit(SceneObjectList *OwnerList);
};

class Scene140: public SceneExt {
	class Action1: public Action {
	public:
		virtual void signal();
	};
public:
	Action1 _action1;
	ASoundExt _soundExt1;
	NamedObject _object1;
	NamedObject _object2;
	IntroSceneText _text;

	void postInit(SceneObjectList *OwnerList);
};

class Scene150: public SceneExt {
	class Action1: public Action {
		NamedObject _object2;
		ASound _sound1;
	public:
		virtual void signal();
	};
public:
	NamedObject _object1;
	Action1 _action1;

	void postInit(SceneObjectList *OwnerList);
};

class Scene160: public SceneExt {
	class Action1: public Action {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
		virtual void process(Event &event);
	};
	class Action3: public ActionExt {
	public:
		virtual void signal();
	};
public:
	NamedObject _flag, _kid, _kidBody, _leftOfficer, _grandma, _rightOfficer;
	ASound _sound1;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	IntroSceneText _text;

	void postInit(SceneObjectList *OwnerList);
};

class Scene180: public SceneExt {
	/* Objects */
	class Vechile: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Items */
	class GarageExit: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	SequenceManager _sequenceManager;
	SpeakerGameText _gameTextSpeaker;
	NamedObject _object1;
	Vechile _vechile;
	NamedHotspot _driveway, _garage, _frontDoor, _house, _street;
	NamedHotspot _lawn, _bushes, _palms, _fence, _steps;
	NamedHotspot _curb, _sky;
	GarageExit _garageExit;
	ASoundExt _sound1;
	SceneMessage _sceneMessage;
	int _dispatchMode;

	Scene180();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene190: public SceneExt {
	/* Objects */
	class LyleCar: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Items */
	class Item1: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item2: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Exit: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager;
	FollowerObject _object1;
	NamedObject _door, _flag;
	LyleCar _lyleCar;
	Item1 _item1;
	Item2 _item2;
	NamedHotspot _item3, _item4, _item5, _item6;
	NamedHotspot _item7, _item8, _item9, _item10;
	Exit _exit;
	Action1 _action1;
	ASoundExt _sound;
	SpeakerGameText _speaker;
	bool _fieldB52;

	Scene190();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
	virtual void synchronize(Serializer &s);
};

} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
