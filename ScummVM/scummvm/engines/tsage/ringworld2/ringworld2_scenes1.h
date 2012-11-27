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

#ifndef TSAGE_RINGWORLD2_SCENES1_H
#define TSAGE_RINGWORLD2_SCENES1_H

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

class Scene1010 : public SceneExt {
public:
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene1020 : public SceneExt {
public:
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene1100 : public SceneExt {
	class Actor16 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor17 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor18 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

public:
	int _field412, _field414;
	SpeakerSeeker1100 _seekerSpeaker;
	SpeakerQuinn1100 _quinnSpeaker;
	SpeakerChief1100 _chiefSpeaker;
	ScenePalette _palette1;
	NamedHotspot _item1;
	NamedHotspot _item2;
	NamedHotspot _item3;
	NamedHotspot _item4;
	NamedHotspot _item5;
	NamedHotspot _item6;
	NamedHotspot _item7;
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
	SceneActor _actor13;
	SceneActor _actor14;
	SceneActor _actor15;
	BackgroundSceneObject _object1;
	BackgroundSceneObject _object2;
	Actor16 _actor16;
	Actor17 _actor17;
	Actor18 _actor18;
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;
	SequenceManager _sequenceManager3;

	Scene1100();
	void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void dispatch();
	virtual void saveCharacter(int characterIndex);
};

class Scene1200 : public SceneExt {
	class Area1: public SceneArea {
	public:
		class Actor3 : public SceneActorExt {
		public:
			void init(int state);
			virtual bool startAction(CursorType action, Event &event);
		};

		SceneActor _actor2;
		Actor3 _actor3;
		Actor3 _actor4;
		Actor3 _actor5;

		byte _field20;

		Area1();
		void synchronize(Serializer &s);

		virtual void postInit(SceneObjectList *OwnerList = NULL);
		virtual void remove();
		virtual void process(Event &event);
		virtual void proc12(int visage, int stripFrameNum, int frameNum, int posX, int posY);
		virtual void proc13(int resNum, int lookLineNum, int talkLineNum, int useLineNum);
	};

public:
	NamedHotspot _item1;
	SceneActor _actor1;
	Area1 _area1;
	UnkObject1200 _object1;
	SequenceManager _sequenceManager;

	int _field412;
	int _field414;
	int _field416;
	int _field418;
	int _field41A;
	int _field41C;

	Scene1200();
	void synchronize(Serializer &s);

	void sub9DAD6(int indx);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
	virtual void saveCharacter(int characterIndex);
};

class Scene1337 : public SceneExt {
	class unkObj1337sub1: public SceneHotspot {
	public:
		SceneObject _object1;

		int _field34;
		Common::Point _field36;

		unkObj1337sub1();
		void synchronize(Serializer &s);
	};

	class unkObj1337_1: public SceneHotspot {
	public:
		unkObj1337sub1 _arr1[4];
		unkObj1337sub1 _arr2[8];
		unkObj1337sub1 _arr3[1];
		unkObj1337sub1 _arr4[1];

		Common::Point _fieldB94;
		Common::Point _fieldB98;
		Common::Point _fieldB9C;
		Common::Point _fieldBA0;
		int _fieldBA4;

		unkObj1337_1();
		void synchronize(Serializer &s);
	};

	class Action1337: public Action {
	public:
		void subD18B5(int resNum, int stripNum, int frameNum);
		void skipFrames(int32 skipCount);
	};

	class Action1: public Action1337 {
	public:
		void signal();
	};
	class Action2: public Action1337 {
	public:
		void signal();
	};
	class Action3: public Action1337 {
	public:
		void signal();
	};
	class Action4: public Action1337 {
	public:
		void signal();
	};
	class Action5: public Action1337 {
	public:
		void signal();
	};
	class Action6: public Action1337 {
	public:
		void signal();
	};
	class Action7: public Action1337 {
	public:
		void signal();
	};
	class Action8: public Action1337 {
	public:
		void signal();
	};
	class Action9: public Action1337 {
	public:
		void signal();
	};
	class Action10: public Action1337 {
	public:
		void signal();
	};
	class Action11: public Action1337 {
	public:
		void signal();
	};
	class Action12: public Action1337 {
	public:
		void signal();
	};
	class Action13: public Action1337 {
	public:
		void signal();
	};
public:
	typedef void (Scene1337::*FunctionPtrType)();
	FunctionPtrType _unkFctPtr412;

	ASound _aSound1;
	ASound _aSound2;
	BackgroundSceneObject _background1;
	bool _autoplay;
	unkObj1337_1 _arrunkObj1337[4];
	SceneItem _item1;
	SceneObject _object1;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	Action7 _action7;
	Action8 _action8;
	Action9 _action9;
	Action10 _action10;
	Action11 _action11;
	Action12 _action12;
	Action13 _action13;
	unkObj1337sub1 _item2;
	unkObj1337sub1 _item3;
	unkObj1337sub1 _item4;
	BackgroundSceneObject _background2;
	int _field3E24;
	int _field3E26;
	int _field3E28[100];
	unkObj1337sub1 *_field3EF0;
	unkObj1337sub1 *_field3EF4;
	unkObj1337sub1 *_field3EF8;
	unkObj1337sub1 _item5;
	unkObj1337sub1 _item6;
	unkObj1337sub1 _item7;
	unkObj1337sub1 _item8;
	int _field423C;
	int _field423E;
	int _field4240;
	int _field4242;
	int _field4244;
	int _field4246;
	int _field4248;
	int _field424A;
	int _field424C;
	int _field424E;

	SceneObject _arrObject1[8];
	SceneObject _arrObject2[8];

	Scene1337();
	virtual void synchronize(Serializer &s);

	void actionDisplay(int resNum, int lineNum, int x, int y, int arg5, int width, int textMode, int fontNum, int colFG, int colBGExt, int colFGExt);
	void setAnimationInfo(unkObj1337sub1 *subObj);
	void subC20E5();
	void subC20F9();
	void subC2586();
	bool subC264B(int arg1);
	bool subC2687(int arg1);
	int  subC26CB(int arg1, int arg2);
	int  subC2719(int arg1);
	int  subC274D(int arg1);
	int  subC2781(int arg1);
	int  subC27B5(int arg1);
	int  subC27F9(int arg1);
	void subC2835(int arg1);
	bool subC2BF8(unkObj1337sub1 *subObj1, Common::Point pt);
	void subC2C2F();
	void subC318B(int arg1, unkObj1337sub1 *subObj2, int arg3);
	int  subC3257(int arg1);
	bool subC32B1(int arg1, int arg2);
	int  subC331B(int arg1);
	bool subC3386(int arg1, int arg2);
	void subC33C0(unkObj1337sub1 *subObj1, unkObj1337sub1 *subObj2);
	void subC3456(unkObj1337sub1 *subObj1, unkObj1337sub1 *subObj2);
	void subC340B(unkObj1337sub1 *subObj1, unkObj1337sub1 *subObj2);
	void subC34A1(unkObj1337sub1 *subObj1, unkObj1337sub1 *subObj2);
	unkObj1337sub1 *subC34EC(int arg1);
	void subC358E(unkObj1337sub1 *subObj1, int arg2);
	int  subC3E92(int arg1);
	void subC4A39(unkObj1337sub1 *subObj);
	void subC4CD2();
	void subC4CEC();
	void subC51A0(unkObj1337sub1 *subObj1, unkObj1337sub1 *subObj2);
	void displayDialog(int dialogNumb);
	void subPostInit();
	void subCB59B();
	void suggestInstructions();
	void shuffleCards();
	void subCCF26();
	void subCD193();
	void subCDB90(int arg1, Common::Point pt);
	void subCF31D();
	void subCF979();
	void subD026D();
	void subD0281();
	void subD02CA();
	void subD183F(int arg1, int arg2);
	void subD18B5(int resNum, int rlbNum, int arg3);
	int  subD18F5();
	int  subD1917();
	int  subD1940(bool flag);
	void subD195F(int arg1, int arg2);
	void subD1975(int arg1, int arg2);
	void subD1A48(int arg1);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene1500 : public SceneExt {
public:
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void dispatch();
};

class Scene1525 : public SceneExt {
public:
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene1530 : public SceneExt {
public:
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;

	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene1550 : public SceneExt {
	class SceneActor1550 : public SceneActor {
	public:
		void subA4D14(int frameNumber, int strip);
	};

	class UnkObj15501 : public SceneActor {
	public:
		int _fieldA4;
		int _fieldA6;

		UnkObj15501();
		void synchronize(Serializer &s);

		virtual bool startAction(CursorType action, Event &event);
	};

	class UnkObj15502 : public SceneActor {
	public:
		int _fieldA4;

		UnkObj15502();
		void synchronize(Serializer &s);

		virtual bool startAction(CursorType action, Event &event);
		void subA5CDF(int strip);
	};

	class UnkObj15503 : public SceneActor {
	public:
		int _fieldA4;

		UnkObj15503();
		void synchronize(Serializer &s);

		virtual bool startAction(CursorType action, Event &event);
	};

	class UnkArea1550 : public SceneArea {
	public:
		byte _field20;
		SceneActor _areaActor;
		UnkObj15503 _unkObj155031;
		UnkObj15503 _unkObj155032;

		virtual void remove();
		virtual void process(Event &event);
		virtual void proc12(int visage, int stripFrameNum, int frameNum, int posX, int posY);
		virtual void proc13(int resNum, int lookLineNum, int talkLineNum, int useLineNum);
	};

	class Hotspot1 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Hotspot3 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor6 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor7 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor8 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor9 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor10 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor11 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor12 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor13 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor14 : public SceneActor1550 {
		// Nothing specific found in the original
		// TODO: check if it's an useless class
	};

public:
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	Hotspot1 _item1;
	Hotspot1 _item2;
	Hotspot3 _item3;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
	SceneActor _actor5;
	Actor6 _actor6;
	Actor7 _actor7;
	Actor8 _actor8;
	Actor9 _actor9;
	Actor10 _actor10;
	Actor11 _actor11;
	Actor12 _actor12;
	Actor13 _actor13;
	UnkObj15501 _arrUnkObj15501[8];
	Actor14 _actor14;
	Actor14 _actor15;
	Actor14 _actor16;
	Actor14 _actor17;
	Actor14 _actor18;
	Actor14 _actor19;
	UnkObj15502 _arrUnkObj15502[8];
	UnkArea1550 _unkArea1;
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;

	int _field412;
	byte _field414;
	int _field415;
	int _field417;
	int _field419;

	Scene1550();
	void synchronize(Serializer &s);
	void subA2B2F();

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
	virtual void saveCharacter(int characterIndex);
};

class Scene1575 : public SceneExt {
	class Hotspot1 : public NamedHotspot {
	public:
		int _field34;
		int _field36;

		Hotspot1();
		void synchronize(Serializer &s);
		void subA910D(int indx);

		virtual void process(Event &event);
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	int _field412;
	int _field414;
	int _field416;
	int _field418;
	int _field41A;
	Hotspot1 _item1;
	Hotspot1 _item2;
	Hotspot1 _item3;
	Hotspot1 _item4;
	Hotspot1 _item5;
	Hotspot1 _item6;
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
	SceneActor _actor13;
	SceneActor _actor14;
	SceneActor _actor15;
	SceneActor _arrActor[17];
	SequenceManager _sequenceManager1;

	Scene1575();
	void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene1580 : public SceneExt {
	class Hotspot1 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Hotspot2 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor2 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor3 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor4 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor5 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor6 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor7 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	int _field412;
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	Hotspot1 _item1;
	Hotspot2 _item2;
	NamedHotspot _item3;
	SceneActor _actor1;
	SceneActor _arrActor[8];
	Actor2 _actor2;
	Actor3 _actor3;
	Actor4 _actor4;
	Actor5 _actor5;
	Actor6 _actor6;
	Actor7 _actor7;
	SequenceManager _sequenceManager;

	Scene1580();
	void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene1625 : public SceneExt {
	class Actor7 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	int _field412;
	SpeakerMiranda1625 _mirandaSpeaker;
	SpeakerTeal1625 _tealSpeaker;
	SpeakerSoldier1625 _soldierSpeaker;
	NamedHotspot _item1;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
	SceneActor _actor5;
	SceneActor _actor6;
	Actor7 _actor7;
	SequenceManager _sequenceManager;

	Scene1625();
	void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
};

class Scene1700 : public SceneExt {
	class Item2 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor11 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor12 : public SceneActor {
	public:
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
public:
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	NamedHotspot _item1;
	Item2 _item2;
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
	Actor11 _actor11;
	Actor12 _actor12;
	Exit1 _exit1;
	Exit2 _exit2;
	Exit3 _exit3;
	SequenceManager _sequenceManager;

	int _field77A;
	int _field77C;

	Scene1700();
	void synchronize(Serializer &s);
	void subAF3F8();

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene1750 : public SceneExt {
	class Actor4 : public SceneActor {
	public:
		int _fieldA4;
		int _fieldA6;
		int _fieldA8;
		int _fieldAA;
		int _fieldAC;
		int _fieldAE;

		Actor4();
		virtual void synchronize(Serializer &s);
		void subB1A76(int arg1, int arg2, int arg3, int arg4, int arg5);
		void subB1B27();

		virtual void remove();
		virtual void process(Event &event);
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor5 : public SceneActor {
	public:
		int _fieldA4;

		Actor5();
		virtual void synchronize(Serializer &s);

		virtual bool startAction(CursorType action, Event &event);
	};

public:
	NamedHotspot _item1;
	NamedHotspot _item2;
	NamedHotspot _item3;
	NamedHotspot _item4;
	NamedHotspot _item5;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	Actor4 _actor4;
	Actor5 _actor5;
	Actor5 _actor6;
	Actor5 _actor7;
	SequenceManager _sequenceManager;
	PaletteRotation *_rotation;

	int _field412;
	int _field413;
	int _field415;
	int _field417;
	int _field419;
	int _field41B;
	int _field41D;

	Scene1750();
	virtual void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene1800 : public SceneExt {
	class Hotspot5 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor6 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor7 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor8 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Exit1 : public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	int _field412;
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	NamedHotspot _item1;
	NamedHotspot _item2;
	NamedHotspot _item3;
	NamedHotspot _item4;
	Hotspot5 _item5;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
	SceneActor _actor5;
	Actor6 _actor6;
	Actor7 _actor7;
	Actor8 _actor8;
	Actor8 _actor9;
	Exit1 _exit1;
	SequenceManager _sequenceManager;

	Scene1800();
	void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void saveCharacter(int characterIndex);
};

class Scene1850 : public SceneExt {
	class Hotspot2 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor5 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor6 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor8 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

public:
	int _field412;
	int _field414;
	int _field416;
	int _field418;
	Common::Point _field41A;
	int _field41E;
	ScenePalette _palette1;
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	NamedHotspot _item1;
	Hotspot2 _item2;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	SceneActor _actor4;
	Actor5 _actor5;
	Actor6 _actor6;
	Actor6 _actor7;
	Actor8 _actor8;
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;

	Scene1850();
	void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene1875 : public SceneExt {
	class Actor1875 : public SceneActor {
	public:
		int _fieldA4;
		int _fieldA6;

		Actor1875();
		void subB84AB();
		void subB8271(int indx);

		void synchronize(Serializer &s);
		virtual void process(Event &event);
	};
public:
	SpeakerQuinn _quinnSpeaker;
	SpeakerSeeker _seekerSpeaker;
	NamedHotspot _item1;
	NamedHotspot _item2;
	SceneActor _actor1;
	SceneActor _actor2;
	SceneActor _actor3;
	Actor1875 _actor4;
	Actor1875 _actor5;
	Actor1875 _actor6;
	Actor1875 _actor7;
	Actor1875 _actor8;
	SequenceManager _sequenceManager;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
};

class Scene1900 : public SceneExt {
	class Actor2 : public SceneActor {
	public:
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
	SpeakerSeeker1900 _seekerSpeaker;
	NamedHotspot _item1;
	NamedHotspot _item2;
	SceneActor _actor1;
	BackgroundSceneObject _object1;
	BackgroundSceneObject _object2;
	Actor2 _actor2;
	Actor2 _actor3;
	Exit1 _exit1;
	Exit2 _exit2;
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene1925 : public SceneExt {
	class Hotspot2 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Hotspot3 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class ExitUp : public SceneExit {
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
	NamedHotspot _item1;
	Hotspot2 _item2;
	Hotspot3 _item3;
	SceneActor _actor1;
	ExitUp _exitUp;
	Exit2 _exit2;
	Exit3 _exit3;
	Exit4 _exit4;
	SequenceManager _sequenceManager;

	int _field9B8;
	int _levelResNum[5];

	Scene1925();
	void synchronize(Serializer &s);

	void changeLevel(bool upFlag);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene1945 : public SceneExt {
	class Hotspot3 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Hotspot4 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor3 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class ExitUp : public SceneExit {
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
	Hotspot3 _item3;
	Hotspot4 _item4;
	SceneActor _actor1;
	SceneActor _actor2;
	Actor3 _actor3;
	ExitUp _exitUp;
	Exit2 _exit2;
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;

	int _fieldEAA;
	int _fieldEAC;
	CursorType _fieldEAE;

	Scene1945();
	void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene1950 : public SceneExt {
	class Area1: public SceneArea {
	public:
		class Actor10 : public SceneActor {
		public:
			int _fieldA4;
			int _fieldA6;
			int _fieldA8;

			Actor10();
			void synchronize(Serializer &s);

			void init(int indx);
			virtual void process(Event &event);
			virtual bool startAction(CursorType action, Event &event);
		};

		SceneActor _areaActor;
		Actor10 _arrActor1[16];

		byte _field20;
		int _fieldB65;

		Area1();
		void synchronize(Serializer &s);

		virtual void remove();
		virtual void process(Event &event);
		virtual void proc12(int visage, int stripFrameNum, int frameNum, int posX, int posY);
		virtual void proc13(int resNum, int lookLineNum, int talkLineNum, int useLineNum);
	};

	class Hotspot2 : public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	class Actor2 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor3 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor4 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor5 : public SceneActor {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Actor8 : public SceneActor {
	public:
		int _fieldA4;
		int _fieldA6;
		int _fieldA8;
		int _fieldAA;
		int _fieldAC;
		byte _fieldAE;
		byte _fieldAF;

		Actor8();
		void synchronize(Serializer &s);

		virtual void signal();
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
	class Exit4 : public SceneExit {
	public:
		virtual void changeScene();
	};
	class Exit5 : public SceneExit {
	public:
		virtual void changeScene();
	};
	class Exit6 : public SceneExit {
	public:
		virtual void changeScene();
	};
	class Exit7 : public SceneExit {
	public:
		virtual void changeScene();
	};
	class Exit8 : public SceneExit {
	public:
		virtual void changeScene();
	};
public:
	NamedHotspot _item1;
	Hotspot2 _item2;
	SceneActor _actor1;
	BackgroundSceneObject _object1;
	Actor2 _actor2;
	Actor3 _actor3;
	SceneActor _actor4;
	Actor5 _actor5;
	SceneActor _actor6;
	SceneActor _actor7;
	Actor8 _actor8;
	Area1 _area1;
	Exit1 _exit1;
	Exit2 _exit2;
	Exit3 _exit3;
	Exit4 _exit4;
	Exit5 _exit5;
	Exit6 _exit6;
	Exit7 _exit7;
	Exit8 _exit8;
	SequenceManager _sequenceManager;

	int _field412;
	int _field414;
	int _field416;
	Common::Point _field418;
	int _field41C;

	Scene1950();
	void synchronize(Serializer &s);

	void subBDC1E();
	void subBE59B();
	void subBF4B4(int indx);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
};
} // End of namespace Ringworld2
} // End of namespace TsAGE

#endif
