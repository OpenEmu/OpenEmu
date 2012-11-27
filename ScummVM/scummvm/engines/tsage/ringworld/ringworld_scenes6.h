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

#ifndef TSAGE_RINGWORLD_SCENES6_H
#define TSAGE_RINGWORLD_SCENES6_H

#include "common/scummsys.h"
#include "tsage/ringworld/ringworld_logic.h"
#include "tsage/ringworld/ringworld_speakers.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"

namespace TsAGE {

namespace Ringworld {

using namespace TsAGE;

class Scene5000 : public Scene {
	/* Actions */
	class Action1 : public Action {
	public:
		virtual void signal();
		virtual void dispatch();
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
	class Hotspot7 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot8 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class HotspotGroup1 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
public:
	SequenceManager _sequenceManager;
	ASound _soundHandler;
	SpeakerSText _speakerSText;
	SpeakerQText _speakerQText;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	Action6 _action6;
	DisplayHotspot _hotspot1;
	SceneObject _hotspot2, _hotspot3, _hotspot4, _hotspot5, _hotspot6;
	Hotspot7 _hotspot7;
	Hotspot8 _hotspot8;
	HotspotGroup1 _hotspot9, _hotspot10, _hotspot11;
	DisplayHotspot _hotspot12, _hotspot13, _hotspot14, _hotspot15;
	DisplayHotspot _hotspot16, _hotspot17, _hotspot18;

	Scene5000();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene5100 : public Scene {
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
	class HotspotGroup1 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class HotspotGroup2 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot4 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot9 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot17 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot18 : public SceneHotspot {
	public:
		int _index1;
		int _index2;

		virtual void doAction(int action);
	};
	class Hotspot19 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot20 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
public:
	SequenceManager _sequenceManager;
	ASound _soundHandler;
	SpeakerMText _speakerMText;
	SpeakerQText _speakerQText;
	SpeakerSText _speakerSText;
	SpeakerBatText _speakerBatText;
	SpeakerGameText _speakerGameText;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;
	Action5 _action5;
	HotspotGroup1 _hotspot1, _hotspot2, _hotspot3;
	Hotspot4 _hotspot4;
	HotspotGroup2 _hotspot5, _hotspot6, _hotspot7;
	SceneObject _hotspot8;
	Hotspot9 _hotspot9;
	SceneObject _hotspot10, _hotspot11, _hotspot12, _hotspot13, _hotspot14, _hotspot15;
	DisplayHotspot _hotspot16;
	Hotspot17 _hotspot17;
	Hotspot18 _hotspot18;
	Hotspot19 _hotspot19;
	Hotspot20 _hotspot20;
	DisplayHotspot _hotspot21;

	Scene5100();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void dispatch();
};

class Scene5200 : public Scene {
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
	class Hotspot9 : public SceneObjectExt {
	public:
		virtual void doAction(int action);
	};
	class Hotspot10 : public SceneObjectExt {
	public:
		virtual void doAction(int action);
	};
	class Hotspot14 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
public:
	ASound _soundHandler;
	SpeakerFLL _speakerFLL;
	SpeakerFLText _speakerFLText;
	SpeakerQL _speakerQL;
	SpeakerQText _speakerQText;
	SpeakerGameText _speakerGameText;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Action4 _action4;

	SceneObject _hotspot1, _hotspot2, _hotspot3, _hotspot4, _hotspot5, _hotspot6, _hotspot7;
	SceneObject _hotspot8;
	Hotspot9 _hotspot9;
	Hotspot10 _hotspot10;
	DisplayHotspot _hotspot11, _hotspot12, _hotspot13;
	Hotspot14 _hotspot14;

	Scene5200();
	virtual void stripCallback(int v);
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void dispatch();
};

class Scene5300 : public Scene {
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
	class Hotspot1 : public SceneObject {
	public:
		virtual void doAction(int action);
	};
	class Hotspot2 : public SceneObject {
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
public:
	ASound _soundHandler;
	SequenceManager _sequenceManager;
	SpeakerQR _speakerQR;
	SpeakerQL _speakerQL;
	SpeakerQText _speakerQText;
	SpeakerBatR _speakerBatR;
	SpeakerBatText _speakerBatText;
	SpeakerSR _speakerSR;
	SpeakerSL _speakerSL;
	SpeakerSText _speakerSText;
	SpeakerGameText _speakerGameText;
	Action1 _action1;
	Action2 _action2;
	Action3 _action3;
	Hotspot1 _hotspot1;
	Hotspot2 _hotspot2;
	DisplayHotspot _hotspot3;
	SceneObject _hotspot4;
	Hotspot5 _hotspot5;
	Hotspot6 _hotspot6;
	Hotspot7 _hotspot7;
	Hotspot8 _hotspot8;
	int _field1B0A;

	Scene5300();
	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void signal();
	virtual void synchronize(Serializer &s) {
		Scene::synchronize(s);
		s.syncAsSint16LE(_field1B0A);
	}
};

} // End of namespace Ringworld

} // End of namespace TsAGE

#endif
