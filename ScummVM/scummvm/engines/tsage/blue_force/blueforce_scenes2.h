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

#ifndef TSAGE_BLUEFORCE_SCENES2_H
#define TSAGE_BLUEFORCE_SCENES2_H

#include "common/scummsys.h"
#include "tsage/blue_force/blueforce_logic.h"
#include "tsage/blue_force/blueforce_speakers.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/sound.h"

namespace TsAGE {

namespace BlueForce {

using namespace TsAGE;

class IntroObject: public NamedObject {
};

class Scene200: public SceneExt {
	/* Actions */
	class Action1: public ActionExt {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager;
	Action1 _action1;
	Action2 _action2;
	NamedObject _object1, _object2, _object3, _object4, _object5, _object6;
	IntroObject  _object7, _object8, _object9;
	NamedObject _object10, _object11;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
};

class Scene210: public SceneExt {
	/* Actions */
	class Action1: public ActionExt {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager;
	Action1 _action1;
	IntroObject _object1, _object2, _object3, _object4;
	IntroObject _object5, _object6, _object7, _object8;
	NamedObject _object9, _object10, _object11, _object12;
	NamedObject _object13, _object14, _object15;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
};

class Scene220: public SceneExt {
	/* Actions */
	class Action1: public ActionExt {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager;
	Action1 _action1;
	Action2 _action2;
	NamedObject _object1, _object2, _object3, _object4, _object5;
	NamedObject _object6, _object7, _object8, _object9;
	IntroObject _object10, _object11, _object12, _object13;
	IntroObject _object14, _object15, _object16;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
};

class Scene225: public SceneExt {
	/* Actions */
	class Action1: public ActionExt {
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
public:
	SequenceManager _sequenceManager;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	IntroObject _object1, _object2, _object3, _object4;
	IntroObject _object5, _object6, _object7;
	NamedObject _object8, _object9, _object10, _object11, _object12;
	NamedObject _object13, _object14, _object15, _object16;
	NamedObject _object17, _object18, _object19;
	NamedObject _object20, _object21;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
};

class Scene265: public SceneExt {
	/* Actions */
	class Action1: public ActionExt {
	public:
		virtual void signal();
	};
public:
	Action1 _action1;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
};

class Scene270: public SceneExt {
	/* Actions */
	class Action1: public ActionExt {
	public:
		virtual void signal();
	};

	/* Objects */
	class Lyle: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Grandma: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Items */
	class Item: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Exit: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	SequenceManager _sequenceManager1, _sequenceManager2, _sequenceManager3;
	SpeakerGrandma _grandmaSpeaker;
	SpeakerLyle _lyleSpeaker;
	SpeakerJake _jakeSpeaker;
	SpeakerLaura _lauraSpeaker;
	SpeakerSkip _skipSpeaker;
	SpeakerGameText _gameTextSpeaker;
	Action1 _action1;
	NamedObject _object1, _object2, _object3, _laura;
	NamedObject _skip, _tv, _fireplace;
	Lyle _lyle;
	Grandma _grandma;
	Item _couch, _afgan;
	NamedHotspot _appliances;
	NamedHotspot _ivy, _fridge, _photos, _item8, _item9;
	NamedHotspot _item10, _item11, _background;
	Exit _exit;
	int _field380, _field382, _field384, _field386;
	int _field219A, _field21A0;
	Common::Point _tempPos;

	Scene270();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene271: public PalettedScene {
	/* Actions */
	class Action1: public ActionExt {
	public:
		virtual void signal();
	};

	/* Objects */
	class Object12: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Items */
	class Item: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Exit: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	SequenceManager _sequenceManager1, _sequenceManager2, _sequenceManager3;
	SpeakerGrandma _grandmaSpeaker;
	SpeakerLyle _lyleSpeaker;
	SpeakerJake _jakeSpeaker;
	SpeakerLaura _lauraSpeaker;
	SpeakerSkip _skipSpeaker;
	SpeakerGameText _gameTextSpeaker;
	SpeakerGranText _granTextSpeaker;
	SpeakerLyleText _lyleTextSpeaker;

	NamedObject _object1, _object2, _object3, _object4, _object5;
	NamedObject _object6, _object7, _object8, _tv, _object10;
	NamedObject _object11;
	Object12 _object12;
	Item _item1, _item3;
	NamedHotspot _item2, _item4, _item5, _item6, _item7;
	NamedHotspot _item8, _item9, _item10, _item11;
	Exit _exit;
	Action1 _action1;
	Rect _rect1;
	int _field796, _field2E16;
	Common::Point _tempPos;

	Scene271();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene280: public PalettedScene {
	/* Actions */
	class Action1: public ActionExt {
	public:
		virtual void signal();
	};
public:
	Action1 _action1;
	SpeakerGameText _gameTextSpeaker;
	NamedObject _jake, _dad, _mum, _object4;

	void postInit(SceneObjectList *OwnerList);
	virtual void signal();
};

} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
