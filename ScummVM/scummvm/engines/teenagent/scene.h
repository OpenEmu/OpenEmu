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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef TEENAGENT_SCENE_H
#define TEENAGENT_SCENE_H

#include "teenagent/surface.h"
#include "teenagent/actor.h"
#include "teenagent/objects.h"
#include "teenagent/surface.h"
#include "teenagent/surface_list.h"
#include "teenagent/teenagent.h"

#include "common/array.h"
#include "common/list.h"

namespace Common {
struct Event;
}

namespace TeenAgent {

class TeenAgentEngine;

struct SceneEvent {
	enum Type {
	    kNone,                  //0
	    kMessage,
	    kWalk,
	    kPlayAnimation,
	    kPlayActorAnimation,    //4
	    kPauseAnimation,
	    kClearAnimations,
	    kLoadScene,
	    kSetOn,                 //8
	    kSetLan,
	    kPlayMusic,
	    kPlaySound,
	    kEnableObject,          //12
	    kHideActor,
	    kWaitForAnimation,
	    kWaitLanAnimationFrame,
	    kCreditsMessage,        //16
	    kCredits,
	    kTimer,
	    kEffect,
	    kFade,
	    kWait,
	    kSetFlag,
	    kQuit
	} type;

	Common::String message;
	byte color;
	byte slot;
	union {
		uint16 animation;
		uint16 callback;
	};
	uint16 timer;
	byte orientation;
	Common::Point dst;
	byte scene; //fixme: put some of these to the union?
	byte ons;
	byte lan;
	union {
		byte music;
		byte firstFrame;
	};
	union {
		byte sound;
		byte lastFrame;
	};
	byte object;

	SceneEvent(Type type_) :
		type(type_), message(), color(textColorMark), slot(0), animation(0), timer(0), orientation(0), dst(),
		scene(0), ons(0), lan(0), music(0), sound(0), object(0) {}

	void clear() {
		type = kNone;
		message.clear();
		color = textColorMark;
		slot = 0;
		orientation = 0;
		animation = 0;
		timer = 0;
		dst.x = dst.y = 0;
		scene = 0;
		ons = 0;
		lan = 0;
		music = 0;
		sound = 0;
		object = 0;
	}

	inline bool empty() const {
		return type == kNone;
	}

	void dump() const {
		debugC(0, kDebugScene, "event[%d]: \"%s\"[%02x], slot: %d, animation: %u, timer: %u, dst: (%d, %d) [%u], scene: %u, ons: %u, lan: %u, object: %u, music: %u, sound: %u",
		      (int)type, message.c_str(), color, slot, animation, timer, dst.x, dst.y, orientation, scene, ons, lan, object, music, sound
		     );
	}
};

class Scene {
public:
	Scene(TeenAgentEngine *engine);
	~Scene();

	bool intro;

	void init(int id, const Common::Point &pos);
	bool render(bool tickGame, bool tickMark, uint32 messageDelta);
	int getId() const { return _id; }

	void warp(const Common::Point &point, byte orientation = 0);

	void moveTo(const Common::Point &point, byte orientation = 0, bool validate = false);
	Common::Point getPosition() const { return position; }

	void displayMessage(const Common::String &str, byte color = textColorMark, const Common::Point &pos = Common::Point());
	void setOrientation(uint8 o) { orientation = o; }
	void push(const SceneEvent &event);
	byte peekFlagEvent(uint16 addr) const;
	SceneEvent::Type last_event_type() const { return !events.empty() ? events.back().type : SceneEvent::kNone; }

	bool processEvent(const Common::Event &event);

	void clear();

	byte *getOns(int id);
	byte *getLans(int id);

	bool eventRunning() const { return !currentEvent.empty(); }

	Walkbox *getWalkbox(byte id) { return &walkboxes[_id - 1][id]; }
	Object *getObject(int id, int sceneId = 0);
	Object *findObject(const Common::Point &point);

	void loadObjectData();
	Animation *getAnimation(byte slot);
	inline Animation *getActorAnimation() { return &actorAnimation; }
	inline const Common::String &getMessage() const { return message; }
	void setPalette(unsigned mul);
	int lookupZoom(uint y) const;

private:
	void loadOns();
	void loadLans();

	void playAnimation(byte idx, uint id, bool loop, bool paused, bool ignore);
	void playActorAnimation(uint id, bool loop, bool ignore);

	byte palette[3 * 256];
	void paletteEffect(byte step);
	byte findFade() const;

	Common::Point messagePosition(const Common::String &str, Common::Point pos);
	uint messageDuration(const Common::String &str);

	bool processEventQueue();
	inline bool nextEvent() {
		currentEvent.clear();
		return processEventQueue();
	}
	void clearMessage();

	TeenAgentEngine *_vm;

	int _id;
	Graphics::Surface background;
	SurfaceList on;
	bool onEnabled;
	Surface *ons;
	uint32 onsCount;
	Animation actorAnimation, animation[4], customAnimation[4];
	Common::Rect actorAnimationPosition, animationPosition[4];

	Actor teenagent, teenagentIdle;
	Common::Point position;

	typedef Common::List<Common::Point> Path;
	Path path;
	uint8 orientation;
	bool actorTalking;

	bool findPath(Path &p, const Common::Point &src, const Common::Point &dst) const;

	Common::Array<Common::Array<Object> > objects;
	Common::Array<Common::Array<Walkbox> > walkboxes;
	Common::Array<Common::Array<FadeType> > fades;

	Common::String message;
	Common::Point messagePos;
	byte messageColor;
	uint messageTimer;
	byte messageFirstFrame;
	byte messageLastFrame;
	Animation *messageAnimation;

	typedef Common::List<SceneEvent> EventList;
	EventList events;
	SceneEvent currentEvent;
	bool hideActor;

	uint16 callback, callbackTimer;

	int _fadeTimer;
	byte _fadeOld;

	uint _idleTimer;

	struct Sound {
		byte id, delay;
		Sound(byte i, byte d): id(i), delay(d) {}
	};
	typedef Common::List<Sound> Sounds;
	Sounds sounds;

	struct DebugFeatures {
		enum {
		    kShowBack,
		    kShowLan,
		    kShowOns,
		    kShowOn,
		    kHidePath,
		    kMax
		};
		bool feature[kMax];

		DebugFeatures() {
			for (uint i = 0; i < kMax; ++i) {
				feature[i] = true;
			}
		}
	} debugFeatures;
};

} // End of namespace TeenAgent

#endif
