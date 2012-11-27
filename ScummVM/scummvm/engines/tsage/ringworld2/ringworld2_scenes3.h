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

#ifndef TSAGE_RINGWORLD2_SCENES3_H
#define TSAGE_RINGWORLD2_SCENES3_H

#include "common/scummsys.h"
#include "tsage/converse.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/sound.h"
#include "tsage/ringworld2/ringworld2_logic.h"
#include "tsage/ringworld2/ringworld2_speakers.h"

namespace TsAGE {

namespace Ringworld2 {

using namespace TsAGE;


class Scene3100 : public SceneExt {
	class Guard : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
public:

	int _field412;
	SpeakerGuard _guardSpeaker;
	NamedHotspot _item1;
	NamedHotspot _item2;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
	SceneActor _actor5;
	Guard _guard;
	ASoundExt _sound1;
	SequenceManager _sequenceManager;

	Scene3100();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void dispatch();
	virtual void synchronize(Serializer &s);
};

class Scene3125 : public SceneExt {
	class Item1 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item2 : public Item1 {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item3 : public Item1 {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor1 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
public:

	int _field412;
	Item1 _item1;
	Actor1 _actor1;
	Item2 _item2;
	Item3 _item3;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
	SceneActor _actor5;
	SequenceManager _sequenceManager1;
	// Second sequence manager... Unused?
	SequenceManager _sequenceManager2;

	Scene3125();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
	virtual void synchronize(Serializer &s);
};

class Scene3150 : public SceneExt {
	class Item5 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item6 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor4 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor5 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor6 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor7 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};

	class Exit1 : public SceneExit {
	public:
		virtual void changeScene();
	};
	class Exit2 : public SceneExit {
	public:
		virtual void changeScene();
	};
public:

	NamedHotspot _item1;
	NamedHotspot _item2;
	NamedHotspot _item3;
	NamedHotspot _item4;
	Item5 _item5;
	Item6 _item6;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	Actor4 _actor4;
	Actor5 _actor5;
	Actor6 _actor6;
	Actor7 _actor7;
	Exit1 _exit1;
	Exit2 _exit2;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene3175 : public SceneExt {
	class Item1 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor3 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor1 : public Actor3 {
		virtual bool startAction(CursorType action, Event &event);
	};
public:

	Item1 _item1;
	Item1 _item2;
	Item1 _item3;
	Actor1 _actor1;
	SceneActor _actor2;
	Actor3 _actor3;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene3200 : public SceneExt {
public:
	SpeakerRocko3200 _rockoSpeaker;
	SpeakerJocko3200 _jockoSpeaker;
	SpeakerSocko3200 _sockoSpeaker;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene3210 : public SceneExt {
public:
	SpeakerCaptain3210 _captainSpeaker;
	SpeakerPrivate3210 _privateSpeaker;
	SceneActor _actor1;
	SceneActor _actor2;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene3220 : public SceneExt {
public:
	SpeakerRocko3220 _rockoSpeaker;
	SpeakerJocko3220 _jockoSpeaker;
	SceneActor _actor1;
	SceneActor _actor2;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene3230 : public SceneExt {
public:
	SpeakerRocko3230 _rockoSpeaker;
	SpeakerJocko3230 _jockoSpeaker;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene3240 : public SceneExt {
public:
	SpeakerTeal3240 _tealSpeaker;
	SpeakerWebbster3240 _webbsterSpeaker;
	SpeakerMiranda _mirandaSpeaker;
	SceneActor _actor1;
	SceneActor _actor2;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene3245 : public SceneExt {
public:
	SpeakerRalf3245 _ralfSpeaker;
	SpeakerTomko3245 _tomkoSpeaker;
	SceneActor _actor1;
	SceneActor _actor2;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene3250 : public SceneExt {
	class Item : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
public:

	Item _item1;
	Item _item2;
	Item _item3;
	Item _item4;
	Actor _actor1;
	Actor _actor2;
	Actor _actor3;
	Actor _actor4;
	SceneActor _actor5;
	SceneActor _actor6;
	SceneActor _actor7;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene3255 : public SceneExt {
public:
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
	SceneActor _actor5;
	SceneActor _actor6;
	SceneActor _actor7;
	SpeakerQuinn3255 _quinnSpeaker;
	SpeakerMiranda3255 _mirandaSpeaker;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene3260 : public SceneExt {
	class Actor13 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor14 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};

	class Action1: public Action {
	public:
		void signal();
	};
public:

	NamedHotspot _item1;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
	SceneActor _actor5;
	SceneActor _actor6;
	SceneActor _actor7;
	SceneActor _actor8;
	SceneActor _actor9;
	SceneActor _actor10;
	SceneActor _actor11;
	SceneActor _actor12;
	Actor13 _actor13;
	Actor14 _actor14;
	Action1 _action1;
	Action1 _action2;
	Action1 _action3;
	Action1 _action4;
	Action1 _action5;
	Action1 _action6;
	Action1 _action7;
	Action1 _action8;
	Action1 _action9;
	Action1 _action10;
	Action1 _action11;
	Action1 _action12;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene3275 : public SceneExt {
	class Actor2 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};

	class Exit1 : public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	NamedHotspot _item1;
	NamedHotspot _item2;
	NamedHotspot _item3;
	NamedHotspot _item4;
	NamedHotspot _item5;
	SceneActor _actor1;
	Actor2 _actor2;
	Exit1 _exit1;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene3350 : public SceneExt {
public:
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
	SceneActor _actor5;
	SceneActor _actor6;
	SceneActor _actor7;
	SceneActor _actor8;
	SceneActor _actor9;
	SequenceManager _sequenceManager;
	PaletteRotation *_rotation;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene3375 : public SceneExt {
	class Actor1 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor2 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor3 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor4 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};

	class Exit1 : public SceneExit {
	public:
		virtual void changeScene();
	};
	class Exit2 : public SceneExit {
	public:
		virtual void changeScene();
	};
	class Exit3 : public SceneExit {
	public:
		virtual void changeScene();
	};

	void signalCase3379();
	void subFC696(int sceneMode);

public:
	SpeakerQuinn3375 _quinnSpeaker;
	SpeakerSeeker3375 _seekerSpeaker;
	SpeakerMiranda3375 _mirandaSpeaker;
	SpeakerWebbster3375 _webbsterSpeaker;
	NamedHotspot _item1;
	NamedHotspot _itemArray[13];
	Actor1 _actor1;
	Actor2 _actor2;
	Actor3 _actor3;
	Actor4 _actor4;
	Exit1 _exit1;
	Exit2 _exit2;
	Exit3 _exit3;
	SequenceManager _sequenceManager;
	int _field1488;
	int _field148A[4];
	int _field1492;

	Scene3375();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void dispatch();
	virtual void synchronize(Serializer &s);
};

class Scene3385 : public SceneExt {
	class Actor1 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor2 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor3 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor4 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};

	class Exit1 : public SceneExit {
	public:
		virtual void changeScene();
	};

	class Action1: public Action {
	public:
		virtual void signal();
	};

public:
	SpeakerQuinn3385 _quinnSpeaker;
	SpeakerSeeker3385 _seekerSpeaker;
	SpeakerMiranda3385 _mirandaSpeaker;
	SpeakerWebbster3385 _webbsterSpeaker;
	NamedHotspot _item1;
	Actor1 _actor1;
	Actor2 _actor2;
	Actor3 _actor3;
	Actor4 _actor4;
	Exit1 _exit1;
	Action1 _action1;
	SequenceManager _sequenceManager;

	int _field11B2;

	Scene3385();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void synchronize(Serializer &s);
};

class Scene3395 : public SceneExt {
	class Actor1 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor2 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor3 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor4 : public SceneActor {
		virtual bool startAction(CursorType action, Event &event);
	};

	class Action1: public Action {
	public:
		virtual void signal();
	};

public:
	SpeakerQuinn3395 _quinnSpeaker;
	SpeakerSeeker3395 _seekerSpeaker;
	SpeakerMiranda3395 _mirandaSpeaker;
	SpeakerWebbster3395 _webbsterSpeaker;
	NamedHotspot _item1;
	NamedHotspot _itemArray[13];
	Actor1 _actor1;
	Actor2 _actor2;
	Actor3 _actor3;
	Actor4 _actor4;
	Action1 _action1;
	SequenceManager _sequenceManager;

	int _field142E;

	Scene3395();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void synchronize(Serializer &s);
};

class Scene3400 : public SceneExt {
public:
	SpeakerQuinn3400 _quinnSpeaker;
	SpeakerSeeker3400 _seekerSpeaker;
	SpeakerMiranda3400 _mirandaSpeaker;
	SpeakerWebbster3400 _webbsterSpeaker;
	SpeakerTeal3400 _tealSpeaker;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
	SceneActor _actor5;
	SceneActor _actor6;
	SceneActor _actor7;
	SceneActor _actor8;
	SequenceManager _sequenceManager;
	int16 _field157C;

	Scene3400();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void synchronize(Serializer &s);
};

class Scene3500 : public SceneExt {
	class Action1: public Action {
	public:
		int _field1E;
		int _field20;
		int _field22;
		int _field24;

		Action1();
		virtual void synchronize(Serializer &s);
		void sub108670(int arg1);
		void sub108732(int arg1);
		virtual void signal();
		virtual void dispatch();
	};
	class Action2: public Action {
	public:
		int _field1E;

		Action2();
		virtual void synchronize(Serializer &s);
		void sub10831F(int arg1);

		virtual void signal();
	};

	class Item4 : public NamedHotspot {
	public:
		int _field34;

		Item4();
		virtual void synchronize(Serializer &s);

		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor7 : public SceneActor {
	public:
		int _fieldA4;
		int _fieldA6;
		int _fieldA8;
		int _fieldAA;
		int _fieldAC;
		int _fieldAE;

		Actor7();
		virtual void synchronize(Serializer &s);

		void sub109466(int arg1, int arg2, int arg3, int arg4, int arg5);
		void sub1094ED();
		void sub109663(int arg1);
		void sub109693(Common::Point Pt);

		virtual void process(Event &event);
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor8 : public SceneActor {
	public:
		// TODO: double check if nothing specific is present, then remove this class
	};

	class UnkObject3500 : public UnkObject1200 {
	public:
		int sub1097C9(int arg1);
		int sub1097EF(int arg1);
		int sub109C09(Common::Point pt);
		int sub109C5E(int &x, int &y);
	};
public:
	Action1 _action1;
	Action2 _action2;
	NamedHotspot _item1;
	NamedHotspot _item2;
	NamedHotspot _item3;
	Item4 _item4;
	Item4 _item5;
	Item4 _item6;
	Item4 _item7;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
	SceneActor _actor5;
	SceneActor _actor6;
	Actor7 _actor7;
	Actor8 _actor8;
	Actor8 _actor9;
	ASoundExt _aSound1;
	UnkObject3500 _unkObj1;
	SequenceManager _sequenceManager;

	int _fieldAF8;
	int _fieldB9E;
	PaletteRotation *_rotation;
	int _field126E;
	int _field1270;
	int _field1272;
	int _field1274;
	int _field1276;
	int _field1278;
	int _field127A;
	int _field127C;
	int _field127E;
	int _field1280;
	int _field1282;
	int _field1284;
	int _field1286;

	Scene3500();
	void sub107F71(int arg1);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
	virtual void synchronize(Serializer &s);
};

class Scene3600 : public SceneExt {
	class Action3600: public ActionExt {
	public:
		int _field1E, _field20;

		Action3600();
		virtual void synchronize(Serializer &s);
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};

	class Item5 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor13 : public SceneActorExt {
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	Action3600 _action1;
	Action2 _action2;
	SpeakerQuinn3600 _quinnSpeaker;
	SpeakerSeeker3600 _seekerSpeaker;
	SpeakerMiranda3600 _mirandaSpeaker;
	SpeakerTeal3600 _tealSpeaker;
	SpeakerProtector3600 _protectorSpeaker;
	NamedHotspot _item1;
	NamedHotspot _item2;
	NamedHotspot _item3;
	NamedHotspot _item4;
	Item5 _item5;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
	SceneActor _actor5;
	SceneActor _actor6;
	SceneActor _actor7;
	SceneActor _actor8;
	SceneActor _actor9;
	SceneActor _actor10;
	SceneActor _actor11;
	SceneActor _actor12;
	Actor13 _actor13;
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;
	SequenceManager _sequenceManager3;
	SequenceManager _sequenceManager4;
	ScenePalette _palette1;

	int _field2548;
	int _field254A;
	int _field254C;
	int _field254E;
	bool _field2550;

	Scene3600();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
	virtual void synchronize(Serializer &s);
};

class Scene3700 : public SceneExt {
public:
	SpeakerQuinn3700 _quinnSpeaker;
	SpeakerSeeker3700 _seekerSpeaker;
	SpeakerMiranda3700 _mirandaSpeaker;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
	SceneActor _actor5;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene3800 : public SceneExt {
	class Exit1 : public SceneExit {
	public:
		virtual void changeScene();
	};

	class Exit2 : public SceneExit {
	public:
		virtual void changeScene();
	};

	class Exit3 : public SceneExit {
	public:
		virtual void changeScene();
	};

	class Exit4 : public SceneExit {
	public:
		virtual void changeScene();
	};

public:
	SceneObject _object1;
	SceneObject _object2;
	SceneActor _actor1;
	NamedHotspot _item1;
	Exit1 _exit1;
	Exit2 _exit2;
	Exit3 _exit3;
	Exit4 _exit4;
	Rect _rect1;
	SequenceManager _sequenceManager1;

	int _field412;

	Scene3800();
	void initScene3800();
	void sub110BBD();

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void synchronize(Serializer &s);
};

class Scene3900 : public SceneExt {
	class Exit1 : public SceneExit {
	public:
		virtual void changeScene();
	};

	class Exit2 : public SceneExit {
	public:
		virtual void changeScene();
	};

	class Exit3 : public SceneExit {
	public:
		virtual void changeScene();
	};

	class Exit4 : public SceneExit {
	public:
		virtual void changeScene();
	};

	class Exit5 : public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	SceneActor _actor1;
	NamedHotspot _item1;
	Exit1 _exit1;
	Exit2 _exit2;
	Exit3 _exit3;
	Exit4 _exit4;
	Exit5 _exit5;
	Rect _rect1;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
};

} // End of namespace Ringworld2
} // End of namespace TsAGE

#endif
