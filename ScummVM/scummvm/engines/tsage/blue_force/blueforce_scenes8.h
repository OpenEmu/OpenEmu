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

#ifndef TSAGE_BLUEFORCE_SCENES8_H
#define TSAGE_BLUEFORCE_SCENES8_H

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

class Scene800: public SceneExt {
	/* Actions */
	class Action1 : public Action {
	public:
		virtual void signal();
	};

	/* Items */
	class Item1: public SceneHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item2: public SceneHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Objects */
	class Doorway: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Car1: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Motorcycle: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Lyle: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Car2: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

public:
	Action1 _action1;
	SequenceManager _sequenceManager;
	SpeakerJakeJacket _jakeJacketSpeaker;
	SpeakerLyleHat _lyleHatSpeaker;
	SpeakerGameText _gameTextSpeaker;
	Doorway _doorway;
	Car1 _car1;
	Motorcycle _motorcycle;
	Lyle _lyle;
	Car2 _car2;
	NamedObject _object6;
	Item1 _item1;
	Item2 _item2;
	SceneText _text;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene810: public SceneExt {
	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
	};
	class Action2: public Action {
	public:
		virtual void signal();
	};

	/* Items */
	class Map: public SceneHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Window: public SceneHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Bookcase: public SceneHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class FaxMachine: public SceneHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class GarbageCan: public SceneHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class FileCabinets: public SceneHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class CoffeeMaker: public SceneHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Shelves: public SceneHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class MicroficheReader: public SceneHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item10: public SceneHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Pictures: public SceneHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Item12: public SceneHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Background: public SceneHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Desk: public SceneHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Exit: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Objects */
	class Lyle: public NamedObjectExt {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Chair: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Object3: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class FaxMachineInset: public FocusObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Object5: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Object7: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	Action1 _action1;
	Action2 _action2;
	SequenceManager _sequenceManager1, _sequenceManager2;
	SpeakerGameText _gameTextSpeaker;
	SpeakerJakeJacket _jakeJacketSpeaker;
	SpeakerJakeUniform _jakeUniformSpeaker;
	SpeakerLyleHat _lyleHatSpeaker;
	Lyle _lyle;
	Chair _chair;
	Object3 _object3;
	FaxMachineInset _faxMachineInset;
	Object5 _object5;
	NamedObject _object6;
	Object7 _object7;
	Map _map;
	Window _window;
	Bookcase _bookcase;
	FaxMachine _faxMachine;
	GarbageCan _garbageCan;
	FileCabinets _fileCabinets;
	CoffeeMaker _coffeeMaker;
	Shelves _shelves;
	MicroficheReader _microficheReader;
	Item10 _item10;
	Pictures _pictures;
	Item12 _item12;
	Background _background;
	Desk _desk;
	Exit _exit;
	ASoundExt _sound1;
	Rect _rect1, _rect2, _rect3;
	int _fieldA70, _fieldA72, _fieldA74;

	Scene810();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene820: public SceneExt {
	/* Objects */
	class PowerButton: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class ForwardButton: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class BackButton: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	SequenceManager _sequenceManager;
	ASoundExt _sound1;
	SpeakerGameText _gameTextSpeaker;
	PowerButton _powerButton;
	BackButton _backButton;
	ForwardButton _forwardButton;
	NamedObject _object4, _object5;
	NamedHotspot _item1;
	int _pageNumber;

	Scene820();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene830: public PalettedScene {
	/* Items */
	class SouthEastExit: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class NoteBoard: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Objects */
	class Lyle: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Door: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class RentalBoat: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Object5: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

public:
	SequenceManager _sequenceManager;
	SpeakerGameText _gameTextSpeaker;
	SpeakerJakeJacket _jakeJacketSpeaker;
	SpeakerLyleHat _lyleHatSpeaker;
	NamedObject _object1, _lyle, _object6;
	Door _door;
	RentalBoat _rentalBoat;
	Object5 _object5;
	SouthEastExit _seExit;
	NoteBoard _noteBoard;
	NamedHotspot _item2, _item4, _item5;
	NamedHotspot _item6, _item7, _item8;
	ASoundExt _sound1;
	int _field18A4, _field18A6, _field18A8, _field18AA, _field18AC;

	Scene830();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene840: public PalettedScene {
	/* Items */
	class Coins: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Exit: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Objects */
	class BoatKeysInset: public FocusObject {
		class RentalKeys: public NamedObject {
		public:
			virtual bool startAction(CursorType action, Event &event);
		};
		class WaveKeys: public NamedObject {
		public:
			virtual bool startAction(CursorType action, Event &event);
		};
	public:
		RentalKeys _rentalKeys;
		WaveKeys _waveKeys;
		int _v1B4, _v1B6;

		virtual void postInit(SceneObjectList *OwnerList = NULL);
		virtual void remove();
		virtual void process(Event &event);
		virtual bool startAction(CursorType action, Event &event);
	};
	class BoatKeys: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Carter: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

public:
	SequenceManager _sequenceManager1, _sequenceManager2;
	NamedObject _object1;
	BoatKeysInset _boatKeysInset;
	NamedObject _doors;
	BoatKeys _boatKeys;
	Carter _carter;
	NamedObject _lyle;
	Coins _coins;
	NamedHotspot _item2, _item3, _item4, _item5, _item6;
	NamedHotspot _item7, _item8, _item9, _item10, _item11;
	NamedHotspot _item12, _item13, _item14, _item15;
	NamedHotspot _item16, _item17, _item18;
	Exit _exit;
	SpeakerGameText _gameTextSpeaker;
	SpeakerJakeJacket _jakeJacketSpeaker;
	SpeakerJakeUniform _jakeUniformSpeaker;
	SpeakerCarter _carterSpeaker;
	int _field1ABA, _field1ABC, _field1ABE, _field1AC0;
	int _field1AC2, _field1AC4, _field1AC6;

	Scene840();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene850: public SceneExt {
	/* Timers */
	class Timer1: public Timer {
	public:
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager;
	Timer1 _timer;
	NamedObject _object1;
	ASoundExt _sound1;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
};

class Scene860: public SceneExt {
	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager;
	NamedObject _deadBody, _object2;
	NamedHotspot _item1;
	Action1 _action1;
	Rect _swRect, _neRect, _yachtRect;
	ASoundExt _sound1;
	int _field87E, _field880, _field886, _field888;
	Common::Point _destPos;

	Scene860();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene870: public SceneExt {
	/* Actions */
	class Action1: public Action {
	public:
		virtual void signal();
	};

	/* Objects */
	class Lyle: public NamedObjectExt {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Green: public NamedObjectExt {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class CrateInset: public FocusObject {
		class Jar: public NamedObjectExt {
		public:
			virtual bool startAction(CursorType action, Event &event);
		};
		class Rags: public NamedObjectExt {
		public:
			virtual bool startAction(CursorType action, Event &event);
		};
	private:
		void initContents();
	public:
		Jar _jar;
		Rags _rags;

		virtual void postInit(SceneObjectList *OwnerList = NULL);
		virtual void remove();
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Items */
	class Boat: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Crate: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Exit: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	SequenceManager _sequenceManager;
	SpeakerGameText _gameTextSpeaker;
	SpeakerJakeJacket _jakeJacketSpeaker;
	SpeakerLyleHat _lyleHatSpeaker;
	SpeakerGreen _greenSpeaker;
	Boat _boat;
	Crate _crate;
	Exit _exit;
	NamedObject _yacht;
	Lyle _lyle;
	Green _green;
	NamedObject _object4, _object5, _object6;
	CrateInset _crateInset;
	NamedHotspot _lumber, _firePit, _water, _boulders;
	NamedHotspot _palmTrees, _sand, _farShore, _item11;
	Action1 _action1;
	int _field1662, _field1664;

	Scene870();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();

	void startStrip(int stripNumber);
};

class Scene880: public SceneExt {
	/* Actions */
	class Action1: public Action {
	private:
		static void SequenceManager_callbackProc(int v1, int v2);
	public:
		virtual void signal();
	};

	/* Objects */
	class Object4: public NamedObjectExt {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Items */
	class NorthExit: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class SouthEastExit: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
private:
	static void handleAction(Action *action);
public:
	SequenceManager _sequenceManager1, _sequenceManager2, _sequenceManager3;
	SpeakerGameText _gameTextSpeaker;
	SpeakerJakeJacket _jakeJacketSpeaker;
	SpeakerLyleHat _lyleHatSpeaker;
	Action1 _action1;
	NamedObject _object1, _object2, _object3;
	Object4 _object4;
	NamedObject _object5, _object6, _object7;
	NamedHotspot _background;
	NorthExit _northExit;
	SouthEastExit _seExit;
	int _seqNumber;

	Scene880();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
