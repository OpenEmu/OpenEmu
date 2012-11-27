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

#ifndef TSAGE_RINGWORLD_LOGIC_H
#define TSAGE_RINGWORLD_LOGIC_H

#include "common/scummsys.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"

namespace TsAGE {

namespace Ringworld {

using namespace TsAGE;

class SceneFactory {
public:
	static Scene *createScene(int sceneNumber);
};

class DisplayHotspot : public SceneObject {
private:
	Common::Array<int> _actions;
	bool performAction(int action);
public:
	DisplayHotspot(int regionId, ...);

	virtual void doAction(int action) {
		if (!performAction(action))
			SceneHotspot::doAction(action);
	}
};

class DisplayObject : public SceneObject {
private:
	Common::Array<int> _actions;
	bool performAction(int action);
public:
	DisplayObject(int firstAction, ...);

	virtual void doAction(int action) {
		if (!performAction(action))
			SceneHotspot::doAction(action);
	}
};

class SceneObjectExt : public SceneObject {
public:
	int _state;

	virtual void synchronize(Serializer &s) {
		SceneObject::synchronize(s);
		s.syncAsSint16LE(_state);
	}
	virtual Common::String getClassName() { return "SceneObjectExt"; }
};

class SceneArea : public SavedObject {
public:
	GfxSurface _surface;
	GfxSurface *_savedArea;
	Common::Point _pt;
	int _resNum;
	int _rlbNum;
	int _subNum;
	int _actionId;
	Rect _bounds;
public:
	SceneArea();
	~SceneArea();

	void setup(int resNum, int rlbNum, int subNum, int actionId);
	void draw2();
	void display();
	void restore();

	virtual void synchronize(Serializer &s);
	virtual void draw(bool flag);
	virtual void wait();
};

/*--------------------------------------------------------------------------*/

class RingworldInvObjectList : public InvObjectList {
public:
	InvObject _stunner;
	InvObject _scanner;
	InvObject _stasisBox;
	InvObject _infoDisk;
	InvObject _stasisNegator;
	InvObject _keyDevice;
	InvObject _medkit;
	InvObject _ladder;
	InvObject _rope;
	InvObject _key;
	InvObject _translator;
	InvObject _ale;
	InvObject _paper;
	InvObject _waldos;
	InvObject _stasisBox2;
	InvObject _ring;
	InvObject _cloak;
	InvObject _tunic;
	InvObject _candle;
	InvObject _straw;
	InvObject _scimitar;
	InvObject _sword;
	InvObject _helmet;
	InvObject _items;
	InvObject _concentrator;
	InvObject _nullifier;
	InvObject _peg;
	InvObject _vial;
	InvObject _jacket;
	InvObject _tunic2;
	InvObject _bone;
	InvObject _jar;
	InvObject _emptyJar;
public:
	RingworldInvObjectList();

	virtual Common::String getClassName() { return "RingworldInvObjectList"; }
};

#define RING_INVENTORY (*((::TsAGE::Ringworld::RingworldInvObjectList *)g_globals->_inventory))

class RingworldGame: public Game {
public:
	virtual void start();
	virtual void restart();
	virtual void endGame(int resNum, int lineNum);

	virtual Scene *createScene(int sceneNumber);
	virtual void processEvent(Event &event);
	virtual void rightClick();
	virtual bool canSaveGameStateCurrently();
	virtual bool canLoadGameStateCurrently();
};

class NamedHotspot : public SceneHotspot {
public:
	NamedHotspot();

	virtual void doAction(int action);
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

} // End of namespace Ringworld

} // End of namespace TsAGE

#endif
