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

#ifndef TSAGE_RINGWORLD_SCENES5_H
#define TSAGE_RINGWORLD_SCENES5_H

#include "common/scummsys.h"
#include "tsage/core.h"
#include "tsage/converse.h"
#include "tsage/ringworld/ringworld_logic.h"
#include "tsage/ringworld/ringworld_speakers.h"

namespace TsAGE {

namespace Ringworld {

using namespace TsAGE;

class Scene4000 : public Scene {
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

	/* Hotspots */
	class Miranda : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot8 : public SceneObject {
	private:
		int _ctr;
	public:
		virtual void synchronize(Serializer &s) {
			SceneObject::synchronize(s);
			s.syncAsUint16LE(_ctr);
		}
		virtual void doAction(int action);
	};
	class GuardRock : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Ladder : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class TheTech : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot13 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot : public SceneObject {
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
	class Hotspot23 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
public:
	SequenceManager _sequenceManager1, _sequenceManager2, _sequenceManager3;
	ASound _soundHandler1, _soundHandler2;
	SpeakerQR _speakerQR;
	SpeakerML _speakerML;
	SpeakerMR _speakerMR;
	SpeakerSR _speakerSR;
	SpeakerCHFL _speakerCHFL;
	SpeakerPL _speakerPL;
	SpeakerPText _speakerPText;
	SpeakerQText _speakerQText;
	SpeakerSText _speakerSText;
	SpeakerMText _speakerMText;
	SpeakerCHFR _speakerCHFR;
	SpeakerQL _speakerQL;
	SpeakerCHFText _speakerCHFText;
	SceneObject _smoke1, _hotspot2, _lander, _olo, _hotspot5, _rope;
	Miranda _miranda;
	Hotspot8 _hotspot8;
	GuardRock _guardRock;
	Ladder _ladder;
	DisplayHotspot _forceField;
	TheTech _theTech;
	Hotspot13 _hotspot13;
	Hotspot _hotspot14, _hotspot15, _hotspot16;
	Hotspot17 _hotspot17;
	Hotspot18 _hotspot18;
	DisplayHotspot _hotspot19, _hotspot20, _hotspot21, _hotspot22;
	Hotspot23 _hotspot23;
	DisplayHotspot _hotspot24, _hotspot25, _hotspot26;
	SceneObject _smoke2;
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

	Scene4000();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene4010 : public Scene {
public:
	SequenceManager _sequenceManager;
	SpeakerQText _speakerQText;
	SpeakerSText _speakerSText;
	SpeakerMText _speakerMText;
	SceneObject _hotspot1, _hotspot2;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
};

class Scene4025 : public Scene {
	/* Custom classes */
	class Peg;

	class Hole : public SceneObject {
	public:
		Peg *_pegPtr;
		int _armStrip;
		Common::Point _newPosition;

		virtual void synchronize(Serializer &s);
		virtual void doAction(int action);
	};
	class Peg : public SceneObject {
	public:
		int _pegId;
		int _armStrip;

		Peg() : SceneObject() { _pegId = 0; _armStrip = 3; }
		virtual void synchronize(Serializer &s);
		virtual void doAction(int action);
	};

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
public:
	SequenceManager _sequenceManager;
	GfxButton _gfxButton;
	SceneObject _armHotspot;
	Hole _hole1, _hole2, _hole3, _hole4, _hole5;
	Peg _peg1, _peg2, _peg3, _peg4, _peg5;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Peg *_pegPtr, *_pegPtr2;
	Hole *_holePtr;

	Scene4025();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void synchronize(Serializer &s);
	virtual void remove();
	virtual void signal();
	virtual void process(Event &event);
	virtual void dispatch();
};

class Scene4045 : public Scene {
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

	/* Hotspots */
	class OlloStand : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Miranda : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Necklace : public SceneObject {
	public:
		virtual void doAction(int action);
	};

public:
	SequenceManager _sequenceManager;
	SpeakerQR _speakerQR;
	SpeakerML _speakerML;
	SpeakerPR _speakerPR;
	SpeakerPText _speakerPText;
	SpeakerQText _speakerQText;
	SpeakerQL _speakerQL;
	OlloStand _olloStand;
	Miranda _miranda;
	DisplayHotspot _flame;
	SceneObject _hotspot4, _olloFace;
	Necklace _necklace;
	DisplayHotspot _hotspot7, _hotspot8, _hotspot9, _hotspot10;
	DisplayHotspot _hotspot11, _hotspot12, _hotspot13, _hotspot14;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;

	Scene4045();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void stripCallback(int v);
	virtual void signal();
	virtual void dispatch();
};

class Scene4050 : public Scene {
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

	/* Hotspots */
	class Hotspot15 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot17 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
public:
	SpeakerPText _speakerPText;
	SpeakerQText _speakerQText;
	SpeakerGameText _speakerGameText;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	DisplayHotspot _hotspot1, _hotspot2, _hotspot3, _hotspot4, _hotspot5;
	DisplayHotspot _hotspot6, _hotspot7, _hotspot8, _hotspot9, _hotspot10;
	DisplayHotspot _hotspot11, _hotspot12, _hotspot13;
	SceneObject _hotspot14;
	Hotspot15 _hotspot15;
	SceneObject _hotspot16;
	Hotspot17 _hotspot17;

	Scene4050();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene4100 : public Scene {
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

	/* Hotspots */
	class Hotspot1 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot2 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Miranda : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Ladder : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot14 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
public:
	SequenceManager _sequenceManager;
	SpeakerMText _speakerMText;
	SpeakerML _speakerML;
	SpeakerQText _speakerQText;
	SpeakerQR _speakerQR;
	SpeakerCHFText _speakerCHFText;
	SpeakerCDRText _speakerCDRText;
	SpeakerCDR _speakerCDR;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	Hotspot1 _hotspot1;
	Hotspot2 _hotspot2;
	DisplayHotspot _hotspot3, _hotspot4;
	Miranda _miranda;
	Ladder _ladder;
	DisplayHotspot _hotspot7, _hotspot8, _hotspot9, _hotspot10;
	DisplayHotspot _hotspot11, _hotspot12, _hotspot13;
	Hotspot14 _hotspot14;

	Scene4100();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene4150 : public Scene {
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

	/* Hotspots */
	class HotspotGroup1 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class HotspotGroup3 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class HotspotGroup6 : public SceneObject {
	public:
		virtual void doAction(int action);
	};

	class Hotspot3 : public SceneObject {
	public:
		virtual void doAction(int action);
	};

public:
	SequenceManager _sequenceManager;
	ASound _soundHandler;
	SpeakerQText _speakerQText;
	SpeakerQR _speakerQR;
	SpeakerCDL _speakerCDL;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	DisplayHotspot _hotspot1, _hotspot2;
	Hotspot3 _hotspot3;
	SceneObject _hotspot4;
	HotspotGroup1 _hotspot5, _hotspot6;
	DisplayHotspot _hotspot7, _hotspot8, _hotspot9, _hotspot10, _hotspot11, _hotspot12;
	HotspotGroup3 _hotspot13, _hotspot14, _hotspot15, _hotspot16;
	DisplayHotspot _hotspot17, _hotspot18, _hotspot19, _hotspot20, _hotspot21;
	DisplayHotspot _hotspot22, _hotspot23, _hotspot24;
	HotspotGroup6 _hotspot25, _hotspot26;

	Scene4150();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene4250 : public Scene {
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
	class Hotspot6 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot8 : public SceneObject {
	public:
		virtual void doAction(int action);
	};

public:
	SequenceManager _sequenceManager;
	ASound _soundHandler;
	SpeakerSR _speakerSR;
	SpeakerSL _speakerSL;
	SpeakerSText _speakerSText;
	SpeakerGameText _speakerGameText;
	SpeakerQL _speakerQL;
	SpeakerQR _speakerQR;
	SpeakerQText _speakerQText;
	SpeakerPText _speakerPText;
	SpeakerMText _speakerMText;
	SpeakerFLText _speakerFLText;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Hotspot1 _hotspot1;
	Hotspot2 _hotspot2;
	SceneObject _hotspot3;
	Hotspot4 _hotspot4;
	SceneObject _hotspot5;
	Hotspot6 _hotspot6;
	DisplayHotspot _hotspot7;
	Hotspot8 _hotspot8;

	Scene4250();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene4300 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		virtual void signal();
	};
	class Action2 : public Action {
	public:
		virtual void signal();
	};

	/* Hotspots */
	class Hotspot8 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot9 : public NamedHotspot {
	public:
		virtual void doAction(int action);
	};
	class Hotspot10 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot15 : public SceneObject {
	public:
		virtual void signal();
	};
	class Hotspot16 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot17 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot19 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
public:
	ASound _soundHandler1, _soundHandler2;
	SequenceManager _sequenceManager;
	GfxButton _gfxButton;
	SpeakerQText _speakerQText;
	SpeakerSText _speakerSText;
	SpeakerMText _speakerMText;
	SpeakerFLText _speakerFLText;

	SceneObject _hotspot1, _hotspot2, _hotspot3, _hotspot4;
	SceneObject _hotspot5, _hotspot6, _hotspot7;
	Hotspot8 _hotspot8;
	Hotspot9 _hotspot9;
	Hotspot10 _hotspot10;
	NamedHotspot _hotspot11;
	SceneObject _hotspot12, _hotspot13, _hotspot14;
	Hotspot15 _hotspot15;
	Hotspot16 _hotspot16;
	Hotspot17 _hotspot17;
	DisplayHotspot _hotspot18;
	Hotspot19 _hotspot19;
	Action1 _action1;
	Action2 _action2;

	Scene4300();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void stripCallback(int v);
	virtual void remove();
	virtual void signal();
	virtual void dispatch();
	virtual void process(Event &event);
};

class Scene4301 : public Scene {
	/* Actions */
	class Action1 : public ActionExt {
	public:
		SceneObject _buttonList[6];
		int _field34E;
		int _indexList[6];

		virtual void synchronize(Serializer &s);
		virtual void remove();
		virtual void signal();
		virtual void process(Event &event);
	};

	/* Hotspots */
	class Hotspot4 : public NamedHotspot {
	public:
		virtual void doAction(int action);
	};
	class Hotspot5 : public NamedHotspot {
	public:
		virtual void doAction(int action);
	};

public:
	Common::List<int> _list1;
	SequenceManager _sequenceManager;
	ASound _soundHandler;
	Action1 _action1;
	SceneObject _hotspot1, _hotspot2, _hotspot3;
	Hotspot4 _hotspot4;
	Hotspot5 _hotspot5;
	bool _puzzleDone;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void dispatch();
	virtual void synchronize(Serializer &s) {
		Scene::synchronize(s);
		s.syncAsSint16LE(_puzzleDone);
	}
};

} // End of namespace Ringworld

} // End of namespace TsAGE

#endif
