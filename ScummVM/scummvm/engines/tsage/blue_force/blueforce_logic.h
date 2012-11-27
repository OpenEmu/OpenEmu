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

#ifndef TSAGE_BLUEFORCE_LOGIC_H
#define TSAGE_BLUEFORCE_LOGIC_H

#include "common/scummsys.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"

namespace TsAGE {

namespace BlueForce {

using namespace TsAGE;

#define BF_INVENTORY (*((::TsAGE::BlueForce::BlueForceInvObjectList *)g_globals->_inventory))

class BlueForceGame: public Game {
public:
	virtual void start();
	virtual Scene *createScene(int sceneNumber);
	virtual void rightClick();
	virtual void processEvent(Event &event);
	virtual bool canSaveGameStateCurrently();
	virtual bool canLoadGameStateCurrently();
	virtual void restart();
};

#define OBJ_ARRAY_SIZE 10
class AObjectArray: public EventHandler {
public:
	EventHandler *_objList[OBJ_ARRAY_SIZE];
	bool _inUse;
	int getNewIndex();
public:
	AObjectArray();
	void clear();

	virtual Common::String getClassName() { return "AObjectArray"; }
	virtual void synchronize(Serializer &s);
	virtual void process(Event &event);
	virtual void dispatch();

	void add(EventHandler *obj);
	void remove(EventHandler *obj);
	// The following line prevents compiler warnings about hiding the remove()
	// method from the parent class.
	virtual void remove() { EventHandler::remove(); }
};

class Timer: public EventHandler {
public:
	Action *_tickAction;
	EventHandler *_endHandler;
	uint32 _endFrame;
public:
	Timer();
	void set(uint32 delay, EventHandler *endHandler);

	virtual Common::String getClassName() { return "Timer"; }
	virtual void synchronize(Serializer &s);
	virtual void remove();
	virtual void signal();
	virtual void dispatch();
};

class TimerExt: public Timer {
public:
	Action *_newAction;
public:
	TimerExt();
	void set(uint32 delay, EventHandler *endHandler, Action *action);

	virtual Common::String getClassName() { return "TimerExt"; }
	virtual void synchronize(Serializer &s);
	virtual void remove();
	virtual void signal();
};


class SceneHotspotExt: public SceneHotspot {
public:
	int _state;

	SceneHotspotExt() { _state = 0; }
	virtual Common::String getClassName() { return "SceneHotspotExt"; }
	virtual void synchronize(Serializer &s) {
		SceneHotspot::synchronize(s);
		s.syncAsSint16LE(_state);
	}
};

class SceneItemType2: public SceneHotspot {
public:
	virtual void startMove(SceneObject *sceneObj, va_list va);
};

class NamedObject: public SceneObject {
public:
	virtual Common::String getClassName() { return "NamedObject"; }
	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual bool startAction(CursorType action, Event &event);
};

class NamedObjectExt: public NamedObject {
public:
	int _flag;

	NamedObjectExt() { _flag = 0; }
	virtual Common::String getClassName() { return "NamedObjectExt"; }
	virtual void synchronize(Serializer &s) {
		NamedObject::synchronize(s);
		s.syncAsSint16LE(_flag);
	}
};

class NamedObject2: public NamedObject {
public:
	int _v1, _v2;

	NamedObject2() { _v1 = _v2 = 0; }
	virtual Common::String getClassName() { return "NamedObject2"; }
	virtual void synchronize(Serializer &s) {
		NamedObject::synchronize(s);
		s.syncAsSint16LE(_v1);
		s.syncAsSint16LE(_v2);
	}
};

class CountdownObject: public NamedObject {
public:
	int _countDown;
	CountdownObject();
	void fixCountdown(int mode, ...);

	virtual Common::String getClassName() { return "CountdownObject"; }
	virtual void synchronize(Serializer &s);
	virtual void dispatch();
};

class FollowerObject: public NamedObject {
public:
	SceneObject *_object;
	FollowerObject();

	virtual Common::String getClassName() { return "FollowerObject"; }
	virtual void synchronize(Serializer &s);
	virtual void remove();
	virtual void dispatch();
	virtual void reposition();

	void setup(SceneObject *object, int visage, int frameNum, int yDiff);
};

class FocusObject: public NamedObject {
public:
	int _v90, _v92;
	GfxSurface _img;

	FocusObject();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void synchronize(Serializer &s);
	virtual void remove();
	virtual void process(Event &event);
};

enum ExitFrame { EXITFRAME_N = 1, EXITFRAME_NE = 2, EXITFRAME_E = 3, EXITFRAME_SE = 4,
		EXITFRAME_S = 5, EXITFRAME_SW = 6, EXITFRAME_W = 7, EXITFRAME_NW = 8 };

class SceneExt: public Scene {
private:
	static void startStrip();
	static void endStrip();
public:
	AObjectArray _timerList, _objArray2;
	int _field372;
	bool _savedPlayerEnabled;
	bool _savedUiEnabled;
	bool _savedCanWalk;
	int _field37A;

	EventHandler *_focusObject;
	Visage _cursorVisage;

	Rect _v51C34;
public:
	SceneExt();

	virtual Common::String getClassName() { return "SceneExt"; }
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	virtual void process(Event &event);
	virtual void dispatch();
	virtual void loadScene(int sceneNum);
	virtual void checkGun();

	void addTimer(EventHandler *timer) { _timerList.add(timer); }
	void removeTimer(EventHandler *timer) { _timerList.remove(timer); }
	bool display(CursorType action);
	void fadeOut();
	void gunDisplay();
	void clearScreen();
};

class PalettedScene: public SceneExt {
public:
	ScenePalette _palette;
	int _field794;
public:
	PalettedScene();

	virtual void synchronize(Serializer &s);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void remove();
	PaletteFader *addFader(const byte *arrBufferRGB, int step, Action *action);
	void add2Faders(const byte *arrBufferRGB, int step, int paletteNum, Action *action);
	void transition(const byte *arrBufferRGB, int arg8, int paletteNum, Action *action, int fromColor1, int fromColor2, int toColor1, int toColor2, bool flag);
};

class SceneHandlerExt: public SceneHandler {
public:
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void process(Event &event);

	virtual void playerAction(Event &event);
	virtual void processEnd(Event &event);
};

class BlueForceInvObjectList : public InvObjectList {
private:
	static bool SelectItem(int objectNumber);
public:
	InvObject _none;
	InvObject _colt45;
	InvObject _ammoClip;
	InvObject _spareClip;
	InvObject _handcuffs;
	InvObject _greensGun;
	InvObject _ticketBook;
	InvObject _mirandaCard;
	InvObject _forestRap;
	InvObject _greenId;
	InvObject _baseballCard;
	InvObject _bookingGreen;
	InvObject _flare;
	InvObject _cobbRap;
	InvObject _bullet22;
	InvObject _autoRifle;
	InvObject _wig;
	InvObject _frankieId;
	InvObject _tyroneId;
	InvObject _snub22;
	InvObject _bug;
	InvObject _bookingFrankie;
	InvObject _bookingGang;
	InvObject _fbiTeletype;
	InvObject _daNote;
	InvObject _printOut;
	InvObject _warehouseKeys;
	InvObject _centerPunch;
	InvObject _tranqGun;
	InvObject _hook;
	InvObject _rags;
	InvObject _jar;
	InvObject _screwdriver;
	InvObject _dFloppy;
	InvObject _blankDisk;
	InvObject _stick;
	InvObject _crate1;
	InvObject _crate2;
	InvObject _shoebox;
	InvObject _badge;
	InvObject _bug2;
	InvObject _rentalCoupon;
	InvObject _nickel;
	InvObject _lyleCard;
	InvObject _carterNote;
	InvObject _mugshot;
	InvObject _clipping;
	InvObject _microfilm;
	InvObject _waveKeys;
	InvObject _rentalKeys;
	InvObject _napkin;
	InvObject _dmvPrintout;
	InvObject _fishingNet;
	InvObject _id;
	InvObject _bullets9mm;
	InvObject _schedule;
	InvObject _grenades;
	InvObject _yellowCord;
	InvObject _halfYellowCord;
	InvObject _blackCord;
	InvObject _bug3;
	InvObject _halfBlackCord;
	InvObject _warrant;
	InvObject _jacket;
	InvObject _greensKnife;
	InvObject _dogWhistle;
	InvObject _ammoBelt;
	InvObject _alleyCatKey;

	BlueForceInvObjectList();
	void reset();
	void setObjectScene(int objectNum, int sceneNumber);
	void alterInventory(int mode);

	virtual Common::String getClassName() { return "BlueForceInvObjectList"; }
};

class NamedHotspot : public SceneHotspot {
public:
	NamedHotspot();

	virtual bool startAction(CursorType action, Event &event);
	virtual Common::String getClassName() { return "NamedHotspot"; }
	virtual void synchronize(Serializer &s);
};

class NamedHotspotExt : public NamedHotspot {
public:
	int _flag;
	NamedHotspotExt() { _flag = 0; }

	virtual Common::String getClassName() { return "NamedHotspot"; }
	virtual void synchronize(Serializer &s) {
		NamedHotspot::synchronize(s);
		s.syncAsSint16LE(_flag);
	}
};

class SceneMessage: public Action {
private:
	Common::String _message;

	void draw();
	void clear();
public:
	void setup(const Common::String &msg) { _message = msg; }

	virtual Common::String getClassName() { return "SceneMessage"; }
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
};

class IntroSceneText: public SceneText {
public:
	Action *_action;
	uint32 _frameNumber;
	int _diff;
public:
	IntroSceneText();
	void setup(const Common::String &msg, Action *action);

	virtual Common::String getClassName() { return "BFIntroText"; }
	virtual void synchronize(Serializer &s);
	virtual void dispatch();
};

} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
