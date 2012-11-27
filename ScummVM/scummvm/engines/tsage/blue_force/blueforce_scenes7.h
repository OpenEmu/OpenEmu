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

#ifndef TSAGE_BLUEFORCE_SCENES7_H
#define TSAGE_BLUEFORCE_SCENES7_H

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

class Scene710: public SceneExt {
	// Actions
	class Action1: public ActionExt {
	public:
		void signal();
	};
	// Timers
	class Timer1: public Timer {
	public:
		void signal();
	};
	// Objects
	class Object3: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event);
	};
	class Object4: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event);
	};
	class Object5: public NamedObject {
	public:
		bool startAction(CursorType action, Event &event);
	};

public:
	SequenceManager _sequenceManager1;
	SequenceManager _sequenceManager2;
	SequenceManager _sequenceManager3;
	Timer1 _timer1;
	SpeakerJake _jakeSpeaker;
	SpeakerLaura _lauraSpeaker;
	SpeakerSkip _skipSpeaker;
	SpeakerGameText _gameTextSpeaker;
	Action1 _action1;
	NamedObject _object1;
	NamedObject _kid;
	Object3 _laura;
	Object4 _dog;
	Object5 _stick;
	ASoundExt _soundExt1;
	NamedHotspot _item1;
	NamedHotspot _item2;
	NamedHotspot _item3;
	NamedHotspot _item4;
	NamedHotspot _item5;
	NamedHotspot _item6;
	NamedHotspot _item7;
	NamedHotspot _item8;
	NamedHotspot _item9;
	int _stickThrowCount;
	bool _dogLying;
	bool _watchCrate;
	bool _throwStick;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
	virtual void synchronize(Serializer &s);
};

} // End of namespace BlueForce
} // End of namespace TsAGE

#endif
