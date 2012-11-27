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

#ifndef TSAGE_BLUEFORCE_SCENES5_H
#define TSAGE_BLUEFORCE_SCENES5_H

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

class Scene550: public SceneExt {
	/* Objects */
	class Lyle: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class CaravanDoor: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Vechile: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Actions */
	class Action1: public ActionExt {
	public:
		virtual void signal();
	};
public:
	SequenceManager _sequenceManager;
	Lyle _lyle;
	CaravanDoor _caravanDoor;
	Vechile _vechile;
	NamedHotspot _item1, _item2, _item3;
	SpeakerGameText _gameTextSpeaker;
	SpeakerLyleHat _lyleHatSpeaker;
	SpeakerJakeJacket _jakeJacketSpeaker;
	Action1 _action1;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene551: public Scene550 {
	/* Objects */
	class Vechile: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class DrunkStanding: public NamedObjectExt {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Drunk: public NamedObjectExt {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class PatrolCarTrunk: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class TrunkInset: public FocusObject {
	public:
		virtual void remove();
		virtual bool startAction(CursorType action, Event &event);
	};
	class TrunkKits: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Harrison: public NamedObjectExt {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Items */
	class Item4: public SceneHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Actions */
	class Action2: public Action {
	public:
		virtual void signal();
	};
public:
	SpeakerDrunk _drunkSpeaker;
	SpeakerJakeUniform _jakeUniformSpeaker;
	SpeakerGiggles _gigglesSpeaker;
	Vechile _vechile2;
	DrunkStanding _drunkStanding;
	Drunk _drunk;
	PatrolCarTrunk _patrolCarTrunk;
	TrunkInset _trunkInset;
	TrunkKits _trunkKits;
	Harrison _harrison;
	NamedObject _object11, _object12, _object13;
	SceneObject _object14, _object15;
	Item4 _item4;
	Action2 _action2;
	int _field1CD0, _field1CD2;

	Scene551();
	virtual void synchronize(Serializer &s);
	void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene560: public SceneExt {
	/* Objects */
	class DeskChair: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Box: public NamedObjectExt {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class SafeInset: public FocusObject {
		/* Items */
		class Item: public NamedHotspotExt {
		public:
			virtual bool startAction(CursorType action, Event &event);
		};
	public:
		NamedObject _digit0, _digit1, _digit2;
		Item _item1, _item2, _item3, _item4, _item5, _item6;
		Visage _cursorVisage;

		virtual Common::String getClassName() { return "Scene560_SafeInset"; }
		virtual void postInit(SceneObjectList *OwnerList = NULL);
		virtual void remove();
		virtual void signal();
		virtual void process(Event &event);
	};
	class Nickel: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class BoxInset: public FocusObject {
		/* Items */
		class Item1: public NamedHotspot {
		public:
			virtual bool startAction(CursorType action, Event &event);
		};
	public:
		Item1 _item1;

		virtual Common::String getClassName() { return "Scene560_BoxInset"; }
		virtual void postInit(SceneObjectList *OwnerList = NULL);
		virtual void remove();
	};

	/* Item groups */
	class PicturePart: public NamedHotspotExt {
	public:
		virtual Common::String getClassName() { return "Scene560_PicturePart"; }
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Items */
	class Computer: public NamedHotspot {
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
public:
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	SpeakerGameText _gameTextSpeaker;
	DeskChair _deskChair;
	Box _box;
	SafeInset _safeInset;
	Nickel _nickel;
	BoxInset _boxInset;
	NamedObject _object6;
	PicturePart _picture1, _picture2, _picture3, _picture4;
	Computer _computer;
	NamedHotspot _chair, _lamp, _item4, _trophy, _watercolours, _fileCabinets;
	NamedHotspot _certificate, _bookcase, _desk, _carpet, _item12, _office;
	ASound _sound1;
	bool _field380;
	bool _field11EA;
	Common::Point _destPosition;

	Scene560();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene570: public SceneExt {
	/* Custom classes */
	class PasswordEntry: public EventHandler {
	private:
		void checkPassword();
	public:
		SceneText _passwordText, _entryText;
		Common::String _passwordStr, _entryBuffer;

		PasswordEntry();
		virtual Common::String getClassName() { return "Scene570_PasswordEntry"; }
		virtual void synchronize(Serializer &s);
		virtual void postInit(SceneObjectList *OwnerList = NULL);
		virtual void process(Event &event);
	};
	class Icon: public NamedObject {
	public:
		SceneText _sceneText;
		int _iconId, _folderId, _parentFolderId, _mode;
		Common::String _text;

		Icon();
		virtual Common::String getClassName() { return "Scene570_Custom2"; }
		virtual void synchronize(Serializer &s);
		virtual void remove();
		virtual bool startAction(CursorType action, Event &event);

		void setDetails(int iconId, int folderId, int parentFolderId, int unused, const Common::String &msg);
	};
	class IconManager: public EventHandler {
	public:
		NamedObject _object1;
		SynchronizedList<Icon *> _list;
		int _mode, _selectedFolder, _fieldAA, _fieldAC;

		IconManager();
		virtual void remove();

		void setup(int mode);
		void hideList();
		void refreshList();
		void addItem(Icon *item);
	};

	/* Objects */
	class PowerSwitch: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class PrinterIcon: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Object3: public FocusObject {
	public:
		virtual void remove();
	};

	/* Items */
	class FloppyDrive: public NamedHotspot {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	SequenceManager _sequenceManager;
	SpeakerGameText _gameTextSpeaker;
	PasswordEntry _passwordEntry;
	PowerSwitch _powerSwitch;
	PrinterIcon _printerIcon;
	Object3 _object3;
	NamedObjectExt _object4;
	Icon _folder1, _folder2, _folder3, _folder4;
	Icon _icon1, _icon2, _icon3, _icon4, _icon5;
	Icon _icon6, _icon7, _icon8, _icon9;
	IconManager _iconManager;
	FloppyDrive _floppyDrive;
	NamedHotspot _monitor, _item3, _case, _keyboard, _desk;
	NamedHotspot _item7, _printer, _window, _plant, _item11;
	ASound _sound1;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
};

class Scene580: public SceneExt {
	/* Objects */
	class Vechile: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Door: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Lyle: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
public:
	SequenceManager _sequenceManager;
	Vechile _vechile;
	Door _door;
	Lyle _lyle;
	NamedHotspot _item1;
	SpeakerGameText _gameTextSpeaker;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
};

class Scene590: public PalettedScene {
	/* Objects */
	class Laura: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};
	class Skip: public NamedObject {
	public:
		virtual bool startAction(CursorType action, Event &event);
	};

	/* Items */
	class Exit: public NamedHotspot {
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
public:
	SequenceManager _sequenceManager;
	Laura _laura;
	Skip _skip;
	SceneObject _object3;
	Exit _exit;
	NamedHotspot _item2, _item3, _item4, _item5, _item6, _item7;
	NamedHotspot _item8, _item9, _item10, _item11, _item12;
	SpeakerSkip _skipSpeaker;
	SpeakerJakeJacket _jakeJacketSpeaker;
	SpeakerLaura _lauraSpeaker;
	ASoundExt _sound1;
	Action1 _action1;
	Action2 _action2;
	int _field17DC, _field17DE, _stripNumber, _field17E2;

	Scene590();
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
