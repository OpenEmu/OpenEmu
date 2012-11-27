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

#ifndef TSAGE_BLUEFORCE_SCENES6_H
#define TSAGE_BLUEFORCE_SCENES6_H

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

class Scene600 : public SceneExt {
	/* Actions */
	class Action1 : public Action {
	public:
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager;
	Action1 _action1;
	ASoundExt _sound1;
	NamedObject _ryan, _object2, _skidMarks;
	BackgroundSceneObject _object4, _object5;
	BackgroundSceneObject _object6, _object7, _object8;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void remove();
};

class Scene620 : public PalettedScene {
public:
	SequenceManager _sequenceManager;
	Timer _timer1;
	NamedObject _object1, _object2, _object3;
	NamedObject _object4, _object5, _object6;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene666 : public SceneExt {
	/* Actions */
	class Action1 : public Action {
	public:
		virtual void signal();
	};

	/* Items */
	class Item1: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	Action1 _action1;
	SequenceManager _sequenceManager;
	NamedObject _object1, _object2, _object3;
	Item1 _item1;
	SceneText _text;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};


class Scene690 : public SceneExt {
	/* Objects */
	class Object1: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Object2: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	SequenceManager _sequenceManager;
	Object1 _object1;
	Object2 _object2;
	NamedObject _object3, _object4, _object5, _object6;
	NamedHotspot _item1, _item2, _item3, _item4, _item5;
	NamedHotspot _item6, _item7, _item8, _item9, _item10;
	NamedHotspot _item11, _item12, _item13, _item14, _item15;
	NamedHotspot _item16, _item17;
	SpeakerJake _jakeSpeaker;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};
} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
