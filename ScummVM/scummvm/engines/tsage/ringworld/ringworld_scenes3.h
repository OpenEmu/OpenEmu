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

#ifndef TSAGE_RINGWORLD_SCENES3_H
#define TSAGE_RINGWORLD_SCENES3_H

#include "common/scummsys.h"
#include "tsage/core.h"
#include "tsage/converse.h"
#include "tsage/ringworld/ringworld_logic.h"
#include "tsage/ringworld/ringworld_speakers.h"

namespace TsAGE {

namespace Ringworld {

using namespace TsAGE;

class Scene2000 : public Scene {
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
	};
	class Action9 : public Action {
	public:
		virtual void signal();
	};
	class Action10 : public Action {
	public:
		virtual void signal();
	};
	class Action11 : public Action {
	public:
		virtual void signal();
	};
	class Action12 : public Action {
	public:
		virtual void signal();
	};
	class Action13 : public Action {
	public:
		virtual void signal();
	};
	class Action14 : public Action {
	public:
		virtual void signal();
	};

public:
	SpeakerSL _speakerSL;
	SpeakerQR _speakerQR;
	SpeakerMR _speakerMR;
	SpeakerQText _speakerQText;
	SpeakerMText _speakerMText;
	SpeakerSText _speakerSText;
	SpeakerHText _speakerHText;
	SpeakerGameText _speakerGameText;
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
	Action14 _action14;
	SceneObject _object1, _object2, _object3, _object4, _object5;
	SceneObject _object6, _object7, _object8, _object9, _object10;
	ASound _soundHandler1, _soundHandler2;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void stripCallback(int v);
};

class Scene2100 : public Scene {
	/* Actions */
	class Action1 : public ActionExt {
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
	};
	class Action9 : public Action {
	public:
		virtual void signal();
	};
	class Action10 : public Action {
	public:
		virtual void signal();
	};
	class Action11 : public Action {
	public:
		virtual void signal();
	};
	class Action12 : public Action {
	public:
		virtual void signal();
	};
	class Action13 : public Action {
	public:
		virtual void signal();
	};
	class Action14 : public Action {
	public:
		virtual void signal();
	};
	class Action15 : public Action {
	public:
		virtual void signal();
	};
	class Action16 : public Action {
	public:
		virtual void signal();
	};
	class Action17 : public Action {
	public:
		virtual void signal();
	};

	/* Hotspots */
	class Hotspot2 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot3 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot4 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot8 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot10 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot14 : public SceneObject {
	public:
		virtual void doAction(int action);
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
public:
	SequenceManager _sequenceManager;
	ASound _soundHandler;
	SpeakerMText _speakerMText;
	SpeakerMR _speakerMR;
	SpeakerQL _speakerQL;
	SpeakerQR _speakerQR;
	SpeakerQText _speakerQText;
	SpeakerGameText _speakerGameText;
	SpeakerSText _speakerSText;
	SpeakerSL _speakerSL;
	SpeakerSAL _speakerSAL;
	SpeakerHText _speakerHText;

	DisplayHotspot _hotspot1;
	Hotspot2 _hotspot2;
	Hotspot3 _hotspot3;
	Hotspot4 _hotspot4;
	DisplayHotspot _hotspot5, _hotspot6, _hotspot7;
	Hotspot8 _hotspot8;
	DisplayHotspot _hotspot9;
	Hotspot10 _hotspot10;
	DisplayHotspot _hotspot11, _hotspot12, _hotspot13;
	Hotspot14 _hotspot14;
	DisplayHotspot _hotspot15;

	Object1 _object1;
	Object2 _object2;
	Object3 _object3;
	SceneObject _object4;

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
	Action14 _action14;
	Action15 _action15;
	Action16 _action16;
	Action17 _action17;
	int _sitFl;
	SceneArea _area1, _area2, _area3, _area4;

	Scene2100();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void stripCallback(int v);
	virtual void signal();
	virtual void synchronize(Serializer &s);
};

class Scene2120 : public Scene {
	/* Actions */
	class Entry {
	public:
		int _size;
		int _lineNum;
		int _visage;

		Entry() { _size = 0; _lineNum = 0; _visage = 0; }
		Entry(int size, int lineNum, int visage) { _size = size; _lineNum = lineNum; _visage = visage; }
	};

	class Action1 : public Action {
	private:
		Common::Array<Entry> _entries;
	public:
		Action1();

		virtual void signal();
		virtual void dispatch();
	};

public:
	ASound _soundHandler;
	SceneObject _topicArrowHotspot, _arrowHotspot, _visageHotspot;
	SceneObject _subjectButton, _nextPageButton, _previousPageButton, _exitButton;
	Action1 _action1;
	Rect _listRect;
	int _dbMode, _prevDbMode;
	bool _visageVisable;
	int _subjectIndex;
	int _lineOffset;

	Scene2120();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void synchronize(Serializer &s);
};

class Scene2150 : public Scene {
	/* Actions */
	class Action1 : public ActionExt {
	public:
		virtual void signal();
	};
	class Action2 : public Action {
	public:
		virtual void signal();
	};

	/* Hotspots */
	class Hotspot1 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot2 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot4 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot7 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot10 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
public:
	ASound _soundHandler;
	SequenceManager _sequenceManager;
	SpeakerGameText _speakerGameText;

	Rect _rect1, _rect2;
	Hotspot1 _hotspot1;
	Hotspot2 _hotspot2;
	DisplayHotspot _hotspot3;
	Hotspot4 _hotspot4;
	DisplayHotspot _hotspot5, _hotspot6;
	Hotspot7 _hotspot7;
	DisplayHotspot _hotspot8, _hotspot9;
	Hotspot10 _hotspot10;
	DisplayHotspot _hotspot11;
	SceneObject _hotspot12, _hotspot13, _hotspot14;
	SceneArea _area1, _area2, _area3, _area4;
	Action1 _action1;
	Action2 _action2;

	Scene2150();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void synchronize(Serializer &s);
	virtual void signal();
	virtual void dispatch();
};

class Scene2200 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		virtual void signal();
	};
	class Action2 : public Action {
	public:
		virtual void signal();
	};
	class Action3 : public ActionExt {
	public:
		virtual void signal();
		virtual void process(Event &event);
	};
	class Action4 : public Action {
	public:
		virtual void signal();
	};

	/* Hotspots */
	class Hotspot3 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot5 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot9 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
public:
	SequenceManager _sequenceManager;
	SpeakerMText _speakerMText;
	SpeakerSText _speakerSText;
	SpeakerQText _speakerQText;
	SpeakerSL _speakerSL;
	SpeakerQR _speakerQR;
	SpeakerQL _speakerQL;
	SpeakerMR _speakerMR;
	SpeakerGameText _speakerGameText;
	Rect _exitRect;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	DisplayHotspot _hotspot1;
	Hotspot3 _hotspot3;
	Hotspot5 _hotspot5;
	Hotspot9 _hotspot9;
	DisplayHotspot _hotspot10;
	SceneObject _hotspot2, _hotspot4;
	SceneObject _hotspot6, _hotspot7, _hotspot8;
	ASound _soundHandler1, _soundHandler2;

	Scene2200();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void stripCallback(int v);
	virtual void synchronize(Serializer &s);
	virtual void signal();
	virtual void dispatch();
};

class Scene2222 : public Scene {
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
	ASound _soundHandler;
	SpeakerSText _speakerSText;
	SpeakerMText _speakerMText;
	SpeakerQText _speakerQText;
	SpeakerML _speakerML;
	SpeakerSR _speakerSR;
	Action1 _action1;
	Action2 _action2;
	SceneObject _hotspot1, _hotspot2, _hotspot3, _hotspot4, _hotspot5;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
};

class Scene2230 : public Scene {
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
	};

	/* Hotspots */
	class Hotspot1 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot3 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot4 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot5 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot6 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot7 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot8 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot10 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot11 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot12 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
public:
	ASound _soundHandler;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	Action7 _action7;
	Action8 _action8;
	Hotspot1 _hotspot1;
	Hotspot3 _hotspot3;
	Hotspot4 _hotspot4;
	SceneObject _hotspot2;
	Hotspot5 _hotspot5;
	Hotspot6 _hotspot6;
	Hotspot7 _hotspot7;
	Hotspot8 _hotspot8;
	DisplayHotspot _hotspot9;
	Hotspot10 _hotspot10;
	Hotspot11 _hotspot11;
	Hotspot12 _hotspot12;
	Rect _rect1;
	int _field30A;

	Scene2230();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void synchronize(Serializer &s);
	virtual void dispatch();
};

class Scene2280 : public Scene {
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
	class Action4 : public ActionExt {
	public:
		virtual void signal();
	};

	/* Hotspots */
	class Hotspot1 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot2 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot4 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot7 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot8 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot10 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot12 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot14 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot15 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot16 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot17 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot18 : public SceneObject {
	public:
		virtual void doAction(int action);
	};

public:
	ASound _soundHandler;
	SequenceManager _sequenceManager;
	Rect _exitRect;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Hotspot1 _hotspot1;
	Hotspot2 _hotspot2;
	DisplayHotspot _hotspot3;
	Hotspot4 _hotspot4;
	DisplayHotspot _hotspot5, _hotspot6;
	Hotspot7 _hotspot7;
	Hotspot8 _hotspot8;
	DisplayHotspot _hotspot9;
	Hotspot10 _hotspot10;
	DisplayHotspot _hotspot11;
	Hotspot12 _hotspot12;
	DisplayHotspot _hotspot13;
	Hotspot14 _hotspot14;
	DisplayHotspot _hotspot15, _hotspot16;
	Hotspot17 _hotspot17;
	Hotspot18 _hotspot18;

	Scene2280();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void synchronize(Serializer &s);
	virtual void signal();
	virtual void dispatch();
};

class Scene2300 : public Scene {
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
	class Action4 : public ActionExt {
	public:
		virtual void signal();
	};

	/* Hotspots */
	class Hotspot5 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot7 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
public:
	ASound _soundHandler1, _soundHandler2;
	SpeakerSL _speakerSL;
	SpeakerMText _speakerMText;
	SpeakerQText _speakerQText;
	SpeakerSText _speakerSText;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	SceneObject _hotspot1, _hotspot2, _hotspot3, _hotspot4;
	Hotspot5 _hotspot5;
	SceneObject _hotspot6;
	Hotspot7 _hotspot7;
	SceneObject _hotspot8, _hotspot9, _hotspot10;
	DisplayHotspot _hotspot11, _hotspot12, _hotspot13, _hotspot14, _hotspot15;

	Scene2300();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
};

class Scene2310 : public Scene {
private:
	int findObject(int objIndex);

	/* Custom classes */
	class ProtectionEntry {
	public:
		int _pageNumber;
		int _connectionList[5];

		void set(int pageNumber, int v1, int v2, int v3, int v4, int v5) {
			_pageNumber = pageNumber;
			_connectionList[0] = v1; _connectionList[1] = v2; _connectionList[2] = v3;
			_connectionList[3] = v4; _connectionList[4] = v5;
		}
	};

public:
	SequenceManager _sequenceManager;
	int _wireIndex, _pageIndex;
	SceneObject _wireList[5];
	Rect _rectList[5];
	SceneText _sceneText;
	ProtectionEntry _pageList[21];

	Scene2310();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void synchronize(Serializer &s);
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene2320 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		virtual void signal();
	};
	class Action2 : public Action {
	public:
		virtual void signal();
	};
	class Action3 : public ActionExt {
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
	};

	/* Hotspots */
	class Hotspot5 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot6 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot8 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot10 : public SceneObjectExt {
	public:
		virtual void doAction(int action);
	};
	class Hotspot11 : public SceneObjectExt {
	public:
		virtual void doAction(int action);
	};
	class Hotspot12 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot14 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot15 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
public:
	ASound _soundHandler;
	SequenceManager _sequenceManager1, _sequenceManager2;
	SpeakerMText _speakerMText;
	SpeakerMR _speakerMR;
	SpeakerML _speakerML;
	SpeakerQText _speakerQText;
	SpeakerQL _speakerQL;
	SpeakerQR _speakerQR;
	SpeakerSAL _speakerSAL;
	SpeakerSL _speakerSL;
	SpeakerSR _speakerSR;
	SpeakerSText _speakerSText;
	SpeakerGameText _speakerGameText;
	SceneArea _area1, _area2, _area3, _area4;
	DisplayHotspot _hotspot1, _hotspot2, _hotspot3, _hotspot4;
	Hotspot5 _hotspot5;
	Hotspot6 _hotspot6;
	SceneObject _hotspot7, _hotspot9;
	Hotspot8 _hotspot8;
	Hotspot10 _hotspot10;
	Hotspot11 _hotspot11;
	Hotspot12 _hotspot12;
	DisplayHotspot _hotspot13;
	Hotspot14 _hotspot14;
	Hotspot15 _hotspot15;
	SceneObject _hotspot16;
	SceneItem *_hotspotPtr;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	Action7 _action7;
	Action8 _action8;

	Scene2320();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void synchronize(Serializer &s);
	virtual void signal();
};

class Scene2400 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		virtual void signal();
		virtual void dispatch();
	};
public:
	Action1 _action1;
	SceneObject _object;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
};

} // End of namespace Ringworld

} // End of namespace TsAGE

#endif
