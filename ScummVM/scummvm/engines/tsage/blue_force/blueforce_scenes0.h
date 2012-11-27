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

#ifndef TSAGE_BLUEFORCE_SCENES0_H
#define TSAGE_BLUEFORCE_SCENES0_H

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

class Scene20 : public SceneExt {
	/* Actions */
	class Action1 : public Action {
	private:
		ASoundExt _sound;
	public:
		virtual void signal();
	};
public:
	Action1 _action1;
	ScenePalette _scenePalette;
	SceneObject _tsunamiWave, _letterT, _letterS, _letterU;
	SceneObject _letterN, _letterA, _letterM, _letterI;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
};

class Scene50: public SceneExt {
	class Tooltip: public SavedObject {
	public:
		Rect _bounds;
		Common::String _msg;
		int _newSceneNumber;
		int _locationId;
	public:
		Tooltip();
		void set(const Rect &bounds, int sceneNum, const Common::String &msg, int locationId);
		void update();
		void highlight(bool btnDown);

		virtual Common::String getClassName() { return "Scene50_Tooltip"; }
		virtual void synchronize(Serializer &s);
	};
	class Tooltip2: public Action {
	public:
		Tooltip2(): Action() {}

		virtual Common::String getClassName() { return "Scene50_Tooltip2"; }
		virtual void signal();
		virtual void dispatch();
	};
public:
	int _sceneNumber;
	SceneText _text;
	SceneItemType2 _item;
	Tooltip _location1, _location2, _location3, _location4, _location5;
	Tooltip _location6, _location7, _location8, _location9;
	Timer _timer;
public:
	Scene50();

	virtual Common::String getClassName() { return "Scene50"; }
	virtual void synchronize(Serializer &s);

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
};

class Scene60 : public SceneExt {
	/* Items */
	class Ignition: public NamedHotspot {
	private:
		bool check1();
		bool check2();
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item3: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Radio: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Compartment: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Objects */
	class MirandaCard: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class TicketBook: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class CompartmentDoor: public NamedObject {
	public:
		bool _flag;
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Actions */
	class Action1: public ActionExt {
	private:
		int useRadio();
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
	SequenceManager _sequenceManager;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	NamedObject _object1;
	MirandaCard _mirandaCard;
	TicketBook _ticketBook;
	CompartmentDoor _compartmentDoor;
	SceneObject _dashboard;
	BackgroundSceneObject _car;
	NamedHotspot _item1;
	Ignition _ignition;
	Item3 _item3;
	Radio _radio;
	Compartment _compartment;
	SpeakerGameText _gameTextSpeaker;
	SpeakerJakeRadio _jakeRadioSpeaker;
	ASound _sound;
	int _newScene;
	int _sceneNumber;
	int _visage;
	CursorType _cursorId;
	// TODO: Check if really useless in original
	bool _field1222;

	Scene60();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void dispatch();
};


} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
