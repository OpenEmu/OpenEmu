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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/algorithm.h"
#include "common/ptr.h"
#include "common/textconsole.h"

#include "graphics/palette.h"

#include "teenagent/scene.h"
#include "teenagent/inventory.h"
#include "teenagent/resources.h"
#include "teenagent/surface.h"
#include "teenagent/objects.h"
#include "teenagent/teenagent.h"
#include "teenagent/music.h"

namespace TeenAgent {

Scene::Scene(TeenAgentEngine *vm) : _vm(vm), intro(false), _id(0), ons(0),
	orientation(kActorRight), actorTalking(false), teenagent(vm), teenagentIdle(vm),
	messageTimer(0), messageFirstFrame(0), messageLastFrame(0), messageAnimation(NULL),
	currentEvent(SceneEvent::kNone), hideActor(false), callback(0), callbackTimer(0), _idleTimer(0) {

	_fadeTimer = 0;
	_fadeOld = 0;
	onEnabled = true;

	memset(palette, 0, sizeof(palette));
	background.pixels = 0;

	FilePack varia;
	varia.open("varia.res");

	Common::ScopedPtr<Common::SeekableReadStream> s(varia.getStream(1));
	if (!s)
		error("invalid resource data");

	teenagent.load(*s, Animation::kTypeVaria);
	if (teenagent.empty())
		error("invalid mark animation");

	s.reset(varia.getStream(2));
	if (!s)
		error("invalid resource data");

	teenagentIdle.load(*s, Animation::kTypeVaria);
	if (teenagentIdle.empty())
		error("invalid mark animation");

	varia.close();
	loadObjectData();
}

Scene::~Scene() {
	if (background.pixels)
		background.free();

	delete[] ons;
	ons = 0;
}

void Scene::warp(const Common::Point &_point, byte o) {
	Common::Point point(_point);
	position = point;
	path.clear();
	if (o)
		orientation = o;
}

bool Scene::findPath(Scene::Path &p, const Common::Point &src, const Common::Point &dst) const {
	const Common::Array<Walkbox> &scene_walkboxes = walkboxes[_id - 1];
	if (dst.x < 0 || dst.x >= kScreenWidth || dst.y < 0 || dst.y >= kScreenHeight)
		return false;

	debugC(1, kDebugScene, "findPath %d,%d -> %d,%d", src.x, src.y, dst.x, dst.y);
	p.clear();
	p.push_back(src);
	p.push_back(dst);

	Common::List<uint> boxes;
	for (uint i = 0; i < scene_walkboxes.size(); ++i) {
		const Walkbox &w = scene_walkboxes[i];
		if (!w.rect.in(src) && !w.rect.in(dst))
			boxes.push_back(i);
	}

	for (Path::iterator i = p.begin(); i != p.end() && !boxes.empty();) {
		Path::iterator next = i;
		++next;
		if (next == p.end())
			break;

		const Common::Point &p1 = *i, &p2 = *next;
		debugC(1, kDebugScene, "%d,%d -> %d,%d", p1.x, p1.y, p2.x, p2.y);

		Common::List<uint>::iterator wi;
		for (wi = boxes.begin(); wi != boxes.end(); ++wi) {
			const Walkbox &w = scene_walkboxes[*wi];
			int mask = w.rect.intersects_line(p1, p2);
			if (mask == 0) {
				continue;
			}

			w.dump(1);
			debugC(1, kDebugScene, "%u: intersection mask 0x%04x, searching hints", *wi, mask);
			int dx = p2.x - p1.x, dy = p2.y - p1.y;
			if (dx >= 0) {
				if ((mask & 8) != 0 && w.sideHint[3] != 0) {
					debugC(1, kDebugScene, "hint left: %u", w.sideHint[3]);
					Common::Point w1, w2;
					w.rect.side(w1, w2, w.sideHint[3], p1);
					debugC(1, kDebugScene, "hint: %d,%d-%d,%d", w1.x, w1.y, w2.x, w2.y);
					p.insert(next, w1);
					if (mask & 2)
						p.insert(next, w2);
					boxes.erase(wi);
					break;
				}
			} else {
				if ((mask & 2) != 0 && w.sideHint[1] != 0) {
					debugC(1, kDebugScene, "hint right: %u", w.sideHint[1]);
					Common::Point w1, w2;
					w.rect.side(w1, w2, w.sideHint[1], p1);
					debugC(1, kDebugScene, "hint: %d,%d-%d,%d", w1.x, w1.y, w2.x, w2.y);
					p.insert(next, w1);
					if (mask & 8)
						p.insert(next, w2);
					boxes.erase(wi);
					break;
				}
			}

			if (dy >= 0) {
				if ((mask & 1) != 0 && w.sideHint[0] != 0) {
					debugC(1, kDebugScene, "hint top: %u", w.sideHint[0]);
					Common::Point w1, w2;
					w.rect.side(w1, w2, w.sideHint[0], p1);
					debugC(1, kDebugScene, "hint: %d,%d-%d,%d", w1.x, w1.y, w2.x, w2.y);
					p.insert(next, w1);
					if (mask & 4)
						p.insert(next, w2);
					boxes.erase(wi);
					break;
				}
			} else {
				if ((mask & 4) != 0 && w.sideHint[2] != 0) {
					debugC(1, kDebugScene, "hint bottom: %u", w.sideHint[2]);
					Common::Point w1, w2;
					w.rect.side(w1, w2, w.sideHint[2], p1);
					debugC(1, kDebugScene, "hint: %d,%d-%d,%d", w1.x, w1.y, w2.x, w2.y);
					p.insert(next, w1);
					if (mask & 1)
						p.insert(next, w2);
					boxes.erase(wi);
					break;
				}
			}
		}
		if (wi == boxes.end())
			++i;
	}
	p.pop_front();
	return true;
}

void Scene::moveTo(const Common::Point &_point, byte orient, bool validate) {
	Common::Point point(_point);
	debugC(0, kDebugScene, "moveTo(%d, %d, %u)", point.x, point.y, orient);
	const Common::Array<Walkbox> &scene_walkboxes = walkboxes[_id - 1];

	for (byte i = 0; i < scene_walkboxes.size(); ++i) {
		const Walkbox &w = scene_walkboxes[i];
		if (w.rect.in(point)) {
			debugC(0, kDebugScene, "bumped into walkbox %u", i);
			w.dump();
			byte o = w.orientation;
			switch (o) {
			case 1:
				point.y = w.rect.top - 1;
				break;
			case 2:
				point.x = w.rect.right + 1;
				break;
			case 3:
				point.y = w.rect.bottom + 1;
				break;
			case 4:
				point.x = w.rect.left - 1;
				break;
			default:
				if (validate)
					return;
			}
		}
	}
	if (point == position) {
		if (orient != 0)
			orientation = orient;
		nextEvent();
		return;
	}

	path.clear();
	if (scene_walkboxes.empty()) {
		path.push_back(point);
		return;
	}

	if (!findPath(path, position, point)) {
		_vm->cancel();
		return;
	}

	orientation = orient;
}

void Scene::loadObjectData() {
	//loading objects & walkboxes
	objects.resize(42);
	walkboxes.resize(42);
	fades.resize(42);

	for (byte i = 0; i < 42; ++i) {
		Common::Array<Object> &sceneObjects = objects[i];
		sceneObjects.clear();

		uint16 sceneTable = _vm->res->dseg.get_word(dsAddr_sceneObjectTablePtr + (i * 2));
		uint16 objectAddr;
		while ((objectAddr = _vm->res->dseg.get_word(sceneTable)) != 0) {
			Object obj;
			obj.load(_vm->res->dseg.ptr(objectAddr));
			//obj.dump();
			sceneObjects.push_back(obj);
			sceneTable += 2;
		}
		debugC(0, kDebugScene, "scene[%u] has %u object(s)", i + 1, sceneObjects.size());

		byte *walkboxesBase = _vm->res->dseg.ptr(READ_LE_UINT16(_vm->res->dseg.ptr(dsAddr_sceneWalkboxTablePtr + i * 2)));
		byte walkboxesCount = *walkboxesBase++;
		debugC(0, kDebugScene, "scene[%u] has %u walkboxes", i + 1, walkboxesCount);

		Common::Array<Walkbox> &sceneWalkboxes = walkboxes[i];
		for (byte j = 0; j < walkboxesCount; ++j) {
			Walkbox w;
			w.load(walkboxesBase + 14 * j);
			if ((w.sideHint[0] | w.sideHint[1] | w.sideHint[2] | w.sideHint[3]) == 0) {
				w.sideHint[0] = 2;
				w.sideHint[1] = 3;
				w.sideHint[2] = 4;
				w.sideHint[3] = 1;
			}
			//walkbox[i]->dump();
			sceneWalkboxes.push_back(w);
		}

		byte *fadeTable = _vm->res->dseg.ptr(_vm->res->dseg.get_word(dsAddr_sceneFadeTablePtr + i * 2));
		Common::Array<FadeType> &sceneFades = fades[i];
		while (READ_LE_UINT16(fadeTable) != 0xffff) {
			FadeType fade;
			fade.load(fadeTable);
			fadeTable += 9;
			sceneFades.push_back(fade);
		}
		debugC(0, kDebugScene, "scene[%u] has %u fadeboxes", i + 1, sceneFades.size());
	}
}

Object *Scene::findObject(const Common::Point &point) {
	if (_id == 0)
		return NULL;

	Common::Array<Object> &sceneObjects = objects[_id - 1];

	for (uint i = 0; i < sceneObjects.size(); ++i) {
		Object &obj = sceneObjects[i];
		if (obj.enabled != 0 && obj.rect.in(point))
			return &obj;
	}
	return NULL;
}

byte *Scene::getOns(int id) {
	return _vm->res->dseg.ptr(_vm->res->dseg.get_word(dsAddr_onsAnimationTablePtr + (id - 1) * 2));
}

byte *Scene::getLans(int id) {
	return _vm->res->dseg.ptr(dsAddr_lansAnimationTablePtr + (id - 1) * 4);
}

void Scene::loadOns() {
	debugC(0, kDebugScene, "loading ons animation");

	uint16 addr = _vm->res->dseg.get_word(dsAddr_onsAnimationTablePtr + (_id - 1) * 2);
	debugC(0, kDebugScene, "ons index: %04x", addr);

	onsCount = 0;
	byte b;
	byte onId[16];
	while ((b = _vm->res->dseg.get_byte(addr)) != 0xff) {
		debugC(0, kDebugScene, "on: %04x = %02x", addr, b);
		++addr;
		if (b == 0)
			continue;

		onId[onsCount++] = b;
	}

	delete[] ons;
	ons = NULL;

	if (onsCount > 0) {
		ons = new Surface[onsCount];
		for (uint32 i = 0; i < onsCount; ++i) {
			Common::ScopedPtr<Common::SeekableReadStream> s(_vm->res->ons.getStream(onId[i]));
			if (s) {
				ons[i].load(*s, Surface::kTypeOns);
			}
		}
	}
}

void Scene::loadLans() {
	debugC(0, kDebugScene, "loading lans animation");
	// load lan000

	for (byte i = 0; i < 4; ++i) {
		animation[i].free();

		uint16 bx = dsAddr_lansAnimationTablePtr + (_id - 1) * 4 + i;
		byte bxv = _vm->res->dseg.get_byte(bx);
		uint16 resId = 4 * (_id - 1) + i + 1;
		debugC(0, kDebugScene, "lan[%u]@%04x = %02x, resource id: %u", i, bx, bxv, resId);
		if (bxv == 0)
			continue;

		Common::ScopedPtr<Common::SeekableReadStream> s(_vm->res->loadLan000(resId));
		if (s) {
			animation[i].load(*s, Animation::kTypeLan);
			if (bxv != 0 && bxv != 0xff)
				animation[i].id = bxv;
		}
	}
}

void Scene::init(int id, const Common::Point &pos) {
	debugC(0, kDebugScene, "init(%d)", id);
	_id = id;
	onEnabled = true; //reset on-rendering flag on loading.
	sounds.clear();
	for (byte i = 0; i < 4; ++i)
		customAnimation[i].free();

	if (background.pixels == NULL)
		background.create(kScreenWidth, kScreenHeight, Graphics::PixelFormat::createFormatCLUT8());

	warp(pos);

	_vm->res->loadOff(background, palette, id);
	if (id == 24) {
		// dark scene
		if (_vm->res->dseg.get_byte(dsAddr_lightOnFlag) != 1) {
			// dim down palette
			uint i;
			for (i = 0; i < 624; ++i) {
				palette[i] = palette[i] > 0x20 ? palette[i] - 0x20 : 0;
			}
			for (i = 726; i < 768; ++i) {
				palette[i] = palette[i] > 0x20 ? palette[i] - 0x20 : 0;
			}
		}
	}

	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->res->on.getStream(id));
	int subHack = 0;
	if (id == 7) { // something patched in the captains room
		switch (_vm->res->dseg.get_byte(dsAddr_captainDrawerState)) {
		case 2:
			break;
		case 1:
			subHack = 1;
			break;
		default:
			subHack = 2;
		}
	}
	on.load(*stream, subHack);

	loadOns();
	loadLans();

	// check music
	int nowPlaying = _vm->music->getId();

	if (nowPlaying != _vm->res->dseg.get_byte(dsAddr_currentMusic))
		_vm->music->load(_vm->res->dseg.get_byte(dsAddr_currentMusic));

	_vm->_system->copyRectToScreen(background.pixels, background.pitch, 0, 0, background.w, background.h);
	setPalette(0);
}

void Scene::playAnimation(byte idx, uint id, bool loop, bool paused, bool ignore) {
	debugC(0, kDebugScene, "playAnimation(%u, %u, loop:%s, paused:%s, ignore:%s)", idx, id, loop ? "true" : "false", paused ? "true" : "false", ignore ? "true" : "false");
	assert(idx < 4);
	Common::ScopedPtr<Common::SeekableReadStream> s(_vm->res->loadLan(id + 1));
	if (!s)
		error("playing animation %u failed", id);

	customAnimation[idx].load(*s);
	customAnimation[idx].loop = loop;
	customAnimation[idx].paused = paused;
	customAnimation[idx].ignore = ignore;
}

void Scene::playActorAnimation(uint id, bool loop, bool ignore) {
	debugC(0, kDebugScene, "playActorAnimation(%u, loop:%s, ignore:%s)", id, loop ? "true" : "false", ignore ? "true" : "false");
	Common::ScopedPtr<Common::SeekableReadStream> s(_vm->res->loadLan(id + 1));
	if (!s)
		error("playing animation %u failed", id);

	actorAnimation.load(*s);
	actorAnimation.loop = loop;
	actorAnimation.ignore = ignore;
	actorAnimation.id = id;
}

Animation *Scene::getAnimation(byte slot) {
	assert(slot < 4);
	return customAnimation + slot;
}

byte Scene::peekFlagEvent(uint16 addr) const {
	for (EventList::const_iterator i = events.reverse_begin(); i != events.end(); --i) {
		const SceneEvent &e = *i;
		if (e.type == SceneEvent::kSetFlag && e.callback == addr)
			return e.color;
	}
	return _vm->res->dseg.get_byte(addr);
}

void Scene::push(const SceneEvent &event) {
	debugC(0, kDebugScene, "push");
	//event.dump();
	if (event.type == SceneEvent::kWalk && !events.empty()) {
		SceneEvent &prev = events.back();
		if (prev.type == SceneEvent::kWalk && prev.color == event.color) {
			debugC(0, kDebugScene, "fixing double-move [skipping event!]");
			if ((event.color & 2) != 0) { // relative move
				prev.dst.x += event.dst.x;
				prev.dst.y += event.dst.y;
			} else {
				prev.dst = event.dst;
			}
			return;
		}
	}
	events.push_back(event);
}

bool Scene::processEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN:
		if (!message.empty() && messageFirstFrame == 0) {
			clearMessage();
			nextEvent();
			return true;
		}
		return false;

	case Common::EVENT_KEYDOWN:
		switch (event.kbd.keycode) {
		case Common::KEYCODE_ESCAPE:
		case Common::KEYCODE_SPACE: {
			if (intro && event.kbd.keycode == Common::KEYCODE_ESCAPE) {
				intro = false;
				clearMessage();
				events.clear();
				sounds.clear();
				currentEvent.clear();
				messageColor = textColorMark;
				for (int i = 0; i < 4; ++i)
					customAnimation[i].free();
				_vm->playMusic(4);
				_vm->loadScene(10, Common::Point(136, 153));
				return true;
			}

			if (!message.empty() && messageFirstFrame == 0) {
				clearMessage();
				nextEvent();
				return true;
			}
			break;
		}
#if 0
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (event.kbd.flags & Common::KBD_CTRL) {
				uint feature = event.kbd.keycode - '1';
				if (feature < DebugFeatures::kMax) {
					debugFeatures.feature[feature] = !debugFeatures.feature[feature];
					debugC(0, kDebugScene, "switched feature %u %s", feature, debugFeatures.feature[feature] ? "on" : "off");
				}
			}
			break;
#endif
		default:
			break;
		}

	default:
		return false;
	}
}

struct ZOrderCmp {
	inline bool operator()(const Surface *a, const Surface *b) const {
		return a->y + a->h < b->y + b->h;
	}
};

int Scene::lookupZoom(uint y) const {
	debugC(2, kDebugScene, "lookupZoom(%d)", y);
	for (byte *zoomTable = _vm->res->dseg.ptr(_vm->res->dseg.get_word(dsAddr_sceneZoomTablePtr + (_id - 1) * 2));
	        zoomTable[0] != 0xff && zoomTable[1] != 0xff;
	        zoomTable += 2) {
		debugC(2, kDebugScene, "\t%d %d->%d", y, zoomTable[0], zoomTable[1]);
		if (y <= zoomTable[0]) {
			return 256u * (100 - zoomTable[1]) / 100;
		}
	}
	return 256;
}

void Scene::paletteEffect(byte step) {
	byte *src = _vm->res->dseg.ptr(dsAddr_paletteEffectData);
	byte *dst = palette + (3 * 242);
	for (byte i = 0; i < 13; ++i) {
		for (byte c = 0; c < 3; ++c, ++src)
			*dst++ = *src > step ? *src - step : 0;
	}
}

byte Scene::findFade() const {
	if (_id <= 0)
		return 0;

	const Common::Array<FadeType> &sceneFades = fades[_id - 1];
	for (uint i = 0; i < sceneFades.size(); ++i) {
		const FadeType &fade = sceneFades[i];
		if (fade.rect.in(position)) {
			return fade.value;
		}
	}
	return 0;
}

bool Scene::render(bool tickGame, bool tickMark, uint32 messageDelta) {
	bool busy;
	bool restart;
	uint32 gameDelta = tickGame ? 1 : 0;
	uint32 markDelta = tickMark ? 1 : 0;

	do {
		restart = false;
		busy = processEventQueue();

		if (_fadeTimer && gameDelta != 0) {
			if (_fadeTimer > 0) {
				_fadeTimer -= gameDelta;
				setPalette(_fadeTimer);
			} else {
				_fadeTimer += gameDelta;
				setPalette(_fadeTimer + 4);
			}
		}

		switch (currentEvent.type) {
		case SceneEvent::kCredits: {
			_vm->_system->fillScreen(0);
			// TODO: optimize me
			Graphics::Surface *surface = _vm->_system->lockScreen();
			_vm->res->font7.render(surface, currentEvent.dst.x, currentEvent.dst.y -= gameDelta, currentEvent.message, currentEvent.color);
			_vm->_system->unlockScreen();

			if (currentEvent.dst.y < -(int)currentEvent.timer)
				currentEvent.clear();
			}
			return true;
		default:
			;
		}

		if (!message.empty() && messageTimer != 0) {
			if (messageTimer <= messageDelta) {
				clearMessage();
				nextEvent();
				continue;
			} else
				messageTimer -= messageDelta;
		}

		if (currentEvent.type == SceneEvent::kCreditsMessage) {
			_vm->_system->fillScreen(0);
			Graphics::Surface *surface = _vm->_system->lockScreen();
			if (currentEvent.lan == 8) {
				_vm->res->font8.setShadowColor(currentEvent.orientation);
				_vm->res->font8.render(surface, currentEvent.dst.x, currentEvent.dst.y, message, currentEvent.color);
			} else {
				_vm->res->font7.render(surface, currentEvent.dst.x, currentEvent.dst.y, message, textColorCredits);
			}
			_vm->_system->unlockScreen();
			return true;
		}

		if (background.pixels && debugFeatures.feature[DebugFeatures::kShowBack]) {
			_vm->_system->copyRectToScreen(background.pixels, background.pitch, 0, 0, background.w, background.h);
		} else
			_vm->_system->fillScreen(0);

		Graphics::Surface *surface = _vm->_system->lockScreen();

		bool gotAnyAnimation = false;

		if (ons != NULL && debugFeatures.feature[DebugFeatures::kShowOns]) {
			for (uint32 i = 0; i < onsCount; ++i) {
				Surface *s = ons + i;
				if (s != NULL)
					s->render(surface);
			}
		}

		Common::List<Surface *> zOrder;

		for (byte i = 0; i < 4; ++i) {
			Animation *a = customAnimation + i;
			Surface *s = a->currentFrame(gameDelta);
			if (s != NULL) {
				if (!a->ignore)
					busy = true;
				if (!a->paused && !a->loop)
					gotAnyAnimation = true;
			} else {
				a = animation + i;
				if (!customAnimation[i].empty()) {
					debugC(0, kDebugScene, "custom animation ended, restart animation in the same slot.");
					customAnimation[i].free();
					a->restart();
				}
				s = a->currentFrame(gameDelta);
			}

			if (currentEvent.type == SceneEvent::kWaitLanAnimationFrame && currentEvent.slot == i) {
				if (s == NULL) {
					restart |= nextEvent();
					continue;
				}
				int index = a->currentIndex();
				if (index == currentEvent.animation) {
					debugC(0, kDebugScene, "kWaitLanAnimationFrame(%d, %d) complete", currentEvent.slot, currentEvent.animation);
					restart |= nextEvent();
				}
			}

			if (s == NULL)
				continue;

			if (debugFeatures.feature[DebugFeatures::kShowLan])
				zOrder.push_back(s);

			if (a->id == 0)
				continue;

			Object *obj = getObject(a->id);
			if (obj != NULL) {
				obj->rect.left = s->x;
				obj->rect.top = s->y;
				obj->rect.right = s->w + s->x;
				obj->rect.bottom = s->h + s->y;
				obj->rect.save();
				//obj->dump();
			}
		}

		Common::sort(zOrder.begin(), zOrder.end(), ZOrderCmp());
		Common::List<Surface *>::iterator zOrderIter;

		Surface *mark = actorAnimation.currentFrame(gameDelta);
		int horizon = position.y;

		for (zOrderIter = zOrder.begin(); zOrderIter != zOrder.end(); ++zOrderIter) {
			Surface *s = *zOrderIter;
			if (s->y + s->h > horizon)
				break;
			s->render(surface);
		}

		if (mark != NULL) {
			actorAnimationPosition = mark->render(surface);
			if (!actorAnimation.ignore)
				busy = true;
			else
				busy = false;
			gotAnyAnimation = true;
		} else if (!hideActor) {
			actorAnimation.free();
			uint zoom = lookupZoom(position.y);

			byte fade = findFade();
			if (fade != _fadeOld) {
				_fadeOld = fade;
				paletteEffect(fade);
				if (_fadeTimer == 0)
					setPalette(4);
			}

			if (!path.empty()) {
				const Common::Point &destination = path.front();
				Common::Point dp(destination.x - position.x, destination.y - position.y);

				int o;
				if (ABS(dp.x) > ABS(dp.y))
					o = dp.x > 0 ? kActorRight : kActorLeft;
				else {
					o = dp.y > 0 ? kActorDown : kActorUp;
				}

				if (tickMark) {
					int speedX = zoom / 32; // 8 * zoom / 256
					int speedY = (o == kActorDown || o == kActorUp ? 2 : 1) * zoom / 256;
					if (speedX == 0)
						speedX = 1;
					if (speedY == 0)
						speedY = 1;

					position.y += (ABS(dp.y) < speedY ? dp.y : SIGN(dp.y) * speedY);
					position.x += (o == kActorDown || o == kActorUp) ?
					              (ABS(dp.x) < speedY ? dp.x : SIGN(dp.x) * speedY) :
					              (ABS(dp.x) < speedX ? dp.x : SIGN(dp.x) * speedX);
				}

				_idleTimer = 0;
				teenagentIdle.resetIndex();
				actorAnimationPosition = teenagent.render(surface, position, o, markDelta, false, zoom);

				if (tickMark && position == destination) {
					path.pop_front();
					if (path.empty()) {
						if (orientation == 0)
							orientation = o; // save last orientation
						nextEvent();
						gotAnyAnimation = true;
						restart = true;
					}
					busy = true;
				} else
					busy = true;
			} else {
				teenagent.resetIndex();
				_idleTimer += markDelta;
				if (_idleTimer < 50)
					actorAnimationPosition = teenagent.render(surface, position, orientation, 0, actorTalking, zoom);
				else
					actorAnimationPosition = teenagentIdle.renderIdle(surface, position, orientation, markDelta, zoom, _vm->_rnd);
			}
		}

		if (restart) {
			_vm->_system->unlockScreen();
			continue;
		}
		// removed mark == null. In final scene of chapter 2 mark rendered above table.
		// if it'd cause any bugs, add hack here. (_id != 23 && mark == NULL)
		if (onEnabled && debugFeatures.feature[DebugFeatures::kShowOn])
			on.render(surface, actorAnimationPosition);

		for (; zOrderIter != zOrder.end(); ++zOrderIter) {
			Surface *s = *zOrderIter;
			s->render(surface);
		}

		if (!message.empty()) {
			bool visible = true;
			if (messageFirstFrame != 0 && messageAnimation != NULL) {
				int index = messageAnimation->currentIndex() + 1;
				debugC(0, kDebugScene, "message: %s first: %u index: %u", message.c_str(), messageFirstFrame, index);
				if (index < messageFirstFrame)
					visible = false;
				if (index > messageLastFrame) {
					clearMessage();
					visible = false;
				}
			}

			if (visible) {
				_vm->res->font7.render(surface, messagePos.x, messagePos.y, message, messageColor);
				busy = true;
			}
		}

		if (!busy && !restart && tickGame && callbackTimer) {
			if (--callbackTimer == 0) {
				if (_vm->inventory->active())
					_vm->inventory->activate(false);
				_vm->processCallback(callback);
			}
			debugC(0, kDebugScene, "callback timer = %u", callbackTimer);
		}

		//if (!current_event.empty())
		//	current_event.dump();

		if (!debugFeatures.feature[DebugFeatures::kHidePath]) {
			const Common::Array<Walkbox> & sceneWalkboxes = walkboxes[_id - 1];
			for (uint i = 0; i < sceneWalkboxes.size(); ++i) {
				sceneWalkboxes[i].rect.render(surface, 0xd0 + i);
			}

			Common::Point last_p = position;
			for (Path::const_iterator p = path.begin(); p != path.end(); ++p) {
				const Common::Point dp(p->x - last_p.x, p->y - last_p.y);
				if (dp.x != 0) {
					surface->hLine(last_p.x, last_p.y, p->x, 0xfe);
				} else if (dp.y != 0) {
					surface->vLine(last_p.x, last_p.y, p->y, 0xfe);
				}
				last_p = *p;
			}
		}

		_vm->_system->unlockScreen();

		if (currentEvent.type == SceneEvent::kWait) {
			if (currentEvent.timer > messageDelta) {
				busy = true;
				currentEvent.timer -= messageDelta;
			}

			if (currentEvent.timer <= messageDelta)
				restart |= nextEvent();

		}

		if (!restart && currentEvent.type == SceneEvent::kWaitForAnimation && !gotAnyAnimation) {
			debugC(0, kDebugScene, "no animations, nextevent");
			nextEvent();
			restart = true;
		}

		if (busy) {
			_idleTimer = 0;
			teenagentIdle.resetIndex();
		}
	} while (restart);

	for (Sounds::iterator i = sounds.begin(); i != sounds.end();) {
		Sound &sound = *i;
		if (sound.delay == 0) {
			debugC(1, kDebugScene, "sound %u started", sound.id);
			_vm->playSoundNow(sound.id);
			i = sounds.erase(i);
		} else {
			sound.delay -= gameDelta;
			++i;
		}
	}

	return busy;
}

bool Scene::processEventQueue() {
	while (!events.empty() && currentEvent.empty()) {
		debugC(0, kDebugScene, "processing next event");
		currentEvent = events.front();
		events.pop_front();
		switch (currentEvent.type) {

		case SceneEvent::kSetOn: {
			byte onId = currentEvent.ons;
			if (onId != 0) {
				--onId;
				byte *ptr = getOns(currentEvent.scene == 0 ? _id : currentEvent.scene);
				debugC(0, kDebugScene, "on[%u] = %02x", onId, currentEvent.color);
				ptr[onId] = currentEvent.color;
			} else {
				onEnabled = currentEvent.color != 0;
				debugC(0, kDebugScene, "%s on rendering", onEnabled ? "enabling" : "disabling");
			}
			loadOns();
			currentEvent.clear();
		}
		break;

		case SceneEvent::kSetLan: {
			if (currentEvent.lan != 0) {
				debugC(0, kDebugScene, "lan[%u] = %02x", currentEvent.lan - 1, currentEvent.color);
				byte *ptr = getLans(currentEvent.scene == 0 ? _id : currentEvent.scene);
				ptr[currentEvent.lan - 1] = currentEvent.color;
			}
			loadLans();
			currentEvent.clear();
		}
		break;

		case SceneEvent::kLoadScene: {
			if (currentEvent.scene != 0) {
				init(currentEvent.scene, currentEvent.dst);
				if (currentEvent.orientation != 0)
					orientation = currentEvent.orientation;
			} else {
				// special case, empty scene
				background.free();
				on.free();
				delete[] ons;
				ons = NULL;
				for (byte i = 0; i < 4; ++i) {
					animation[i].free();
					customAnimation[i].free();
				}
			}
			currentEvent.clear();
		}
		break;

		case SceneEvent::kWalk: {
			Common::Point dst = currentEvent.dst;
			if ((currentEvent.color & 2) != 0) { // relative move
				dst.x += position.x;
				dst.y += position.y;
			}
			if ((currentEvent.color & 1) != 0) {
				warp(dst, currentEvent.orientation);
				currentEvent.clear();
			} else
				moveTo(dst, currentEvent.orientation);
		}
		break;

		case SceneEvent::kCreditsMessage:
		case SceneEvent::kMessage: {
			message = currentEvent.message;
			messageAnimation = NULL;
			if (currentEvent.firstFrame) {
				messageTimer = 0;
				messageFirstFrame = currentEvent.firstFrame;
				messageLastFrame = currentEvent.lastFrame;
				if (currentEvent.slot > 0) {
					messageAnimation = customAnimation + (currentEvent.slot - 1);
					//else if (!animation[currentEvent.slot].empty())
					//  messageAnimation = animation + currentEvent.slot;
				} else
					messageAnimation = &actorAnimation;
				debugC(0, kDebugScene, "async message %d-%d (slot %u)", messageFirstFrame, messageLastFrame, currentEvent.slot);
			} else {
				messageTimer = currentEvent.timer ? currentEvent.timer * 110 : messageDuration(message);
				messageFirstFrame = messageLastFrame = 0;
			}
			Common::Point p;
			if (currentEvent.dst.x == 0 && currentEvent.dst.y == 0) {
				p = Common::Point((actorAnimationPosition.left + actorAnimationPosition.right) / 2,
				                  actorAnimationPosition.top);
			} else {
				p = currentEvent.dst;
			}

			byte messageSlot = currentEvent.slot;
			if (messageSlot != 0) {
				--messageSlot;
				assert(messageSlot < 4);
				const Surface *s = customAnimation[messageSlot].currentFrame(0);
				if (s == NULL)
					s = animation[messageSlot].currentFrame(0);
				if (s != NULL) {
					p.x = s->x + s->w / 2;
					p.y = s->y;
				} else
					warning("no animation in slot %u", messageSlot);
			}
			messagePos = messagePosition(message, p);
			messageColor = currentEvent.color;

			if (messageFirstFrame)
				currentEvent.clear(); // async message, clearing event
			}
			break;

		case SceneEvent::kPlayAnimation: {
			byte slot = currentEvent.slot & 7; // 0 - mark's
			if (currentEvent.animation != 0) {
				debugC(0, kDebugScene, "playing animation %u in slot %u(%02x)", currentEvent.animation, slot, currentEvent.slot);
				if (slot != 0) {
					--slot;
					assert(slot < 4);
					playAnimation(slot, currentEvent.animation, (currentEvent.slot & 0x80) != 0, (currentEvent.slot & 0x40) != 0, (currentEvent.slot & 0x20) != 0);
				} else
					actorTalking = true;
			} else {
				if (slot != 0) {
					--slot;
					debugC(0, kDebugScene, "cancelling animation in slot %u", slot);
					assert(slot < 4);
					customAnimation[slot].free();
				} else
					actorTalking = true;
			}
			currentEvent.clear();
			}
			break;

		case SceneEvent::kPauseAnimation: {
			byte slot = currentEvent.slot & 7; // 0 - mark's
			if (slot != 0) {
				--slot;
				debugC(1, kDebugScene, "pause animation in slot %u", slot);
				customAnimation[slot].paused = (currentEvent.slot & 0x80) != 0;
			} else {
				actorTalking = false;
			}
			currentEvent.clear();
			}
			break;

		case SceneEvent::kClearAnimations:
			for (byte i = 0; i < 4; ++i)
				customAnimation[i].free();
			actorTalking = false;
			currentEvent.clear();
			break;

		case SceneEvent::kPlayActorAnimation:
			debugC(0, kDebugScene, "playing actor animation %u", currentEvent.animation);
			playActorAnimation(currentEvent.animation, (currentEvent.slot & 0x80) != 0, (currentEvent.slot & 0x20) != 0);
			currentEvent.clear();
			break;

		case SceneEvent::kPlayMusic:
			debugC(0, kDebugScene, "setting music %u", currentEvent.music);
			_vm->setMusic(currentEvent.music);
			_vm->res->dseg.set_byte(dsAddr_currentMusic, currentEvent.music);
			currentEvent.clear();
			break;

		case SceneEvent::kPlaySound:
			debugC(0, kDebugScene, "playing sound %u, delay: %u", currentEvent.sound, currentEvent.color);
			sounds.push_back(Sound(currentEvent.sound, currentEvent.color));
			currentEvent.clear();
			break;

		case SceneEvent::kEnableObject: {
			debugC(0, kDebugScene, "%s object #%u", currentEvent.color ? "enabling" : "disabling", currentEvent.object - 1);
			Object *obj = getObject(currentEvent.object - 1, currentEvent.scene == 0 ? _id : currentEvent.scene);
			obj->enabled = currentEvent.color;
			obj->save();
			currentEvent.clear();
			}
			break;

		case SceneEvent::kHideActor:
			hideActor = currentEvent.color != 0;
			currentEvent.clear();
			break;

		case SceneEvent::kWaitForAnimation:
			debugC(0, kDebugScene, "waiting for the animation");
			break;

		case SceneEvent::kWaitLanAnimationFrame:
			debugC(0, kDebugScene, "waiting for the frame %d in slot %d", currentEvent.animation, currentEvent.slot);
			break;

		case SceneEvent::kTimer:
			callback = currentEvent.callback;
			callbackTimer = currentEvent.timer;
			debugC(0, kDebugScene, "triggering callback %04x in %u frames", callback, callbackTimer);
			currentEvent.clear();
			break;

		case SceneEvent::kEffect:
			_vm->_system->delayMillis(80); // 2 vsyncs
			_vm->_system->setShakePos(8);
			_vm->_system->updateScreen();

			_vm->_system->delayMillis(80); // 2 vsyncs
			_vm->_system->setShakePos(0);
			_vm->_system->updateScreen();

			_vm->_system->delayMillis(80); // 2 vsyncs
			_vm->_system->setShakePos(4);
			_vm->_system->updateScreen();

			_vm->_system->delayMillis(80); // 2 vsyncs
			_vm->_system->setShakePos(0);
			_vm->_system->updateScreen();

			currentEvent.clear();
			break;

		case SceneEvent::kFade:
			_fadeTimer = currentEvent.orientation != 0 ? 5 : -5;
			currentEvent.clear();
			break;

		case SceneEvent::kWait:
			debugC(0, kDebugScene, "wait %u", currentEvent.timer);
			break;

		case SceneEvent::kCredits:
			debugC(0, kDebugScene, "showing credits");
			break;

		case SceneEvent::kQuit:
			debugC(0, kDebugScene, "quit!");
			_vm->quitGame();
			break;

		case SceneEvent::kSetFlag:
			debugC(0, kDebugScene, "async set_flag(%04x, %d)", currentEvent.callback, currentEvent.color);
			_vm->res->dseg.set_byte(currentEvent.callback, currentEvent.color);
			currentEvent.clear();
			break;

		default:
			error("empty/unhandler event[%d]", (int)currentEvent.type);
		}
	}

	if (events.empty()) {
		messageColor = textColorMark;
		hideActor = false;
	}

	return !currentEvent.empty();
}

void Scene::setPalette(unsigned mul) {
	debugC(0, kDebugScene, "setPalette(%u)", mul);
	byte p[3 * 256];

	for (int i = 0; i < 3 * 256; ++i) {
		p[i] = (unsigned)palette[i] * mul;
	}

	_vm->_system->getPaletteManager()->setPalette(p, 0, 256);
}

Object *Scene::getObject(int id, int sceneId) {
	assert(id > 0);

	if (sceneId == 0)
		sceneId = _id;

	if (sceneId == 0)
		return NULL;

	Common::Array<Object> &sceneObjects = objects[sceneId - 1];
	--id;
	if (id >= (int)sceneObjects.size())
		return NULL;

	return &sceneObjects[id];
}

Common::Point Scene::messagePosition(const Common::String &str, Common::Point pos) {
	int lines = 1;
	for (uint i = 0; i < str.size(); ++i)
		if (str[i] == '\n')
			++lines;

	uint w = _vm->res->font7.render(NULL, 0, 0, str, 0);
	uint h = _vm->res->font7.getHeight() * lines + 3;

	pos.x -= w / 2;
	pos.y -= h;

	if (pos.x + w > kScreenWidth)
		pos.x = kScreenWidth - w;
	if (pos.x < 0)
		pos.x = 0;
	if (pos.y + h > kScreenHeight)
		pos.y = kScreenHeight - h;
	if (pos.y < 0)
		pos.y = 0;

	return pos;
}

uint Scene::messageDuration(const Common::String &str) {
	// original game uses static delays: 100-slow, 50, 20 and 1 tick - crazy speed.
	// total delay = total message length * delay / 8 + 60.
	uint totalWidth = str.size();

	int speed = ConfMan.getInt("talkspeed");
	if (speed < 0)
		speed = 60;
	uint delayDelta = 1 + (255 - speed) * 99 / 255;

	uint delay = 60 + (totalWidth * delayDelta) / 8;
	debugC(0, kDebugScene, "delay = %u, delta: %u", delay, delayDelta);
	return delay * 10;
}

void Scene::displayMessage(const Common::String &str, byte color, const Common::Point &pos) {
	//assert(!str.empty());
	debugC(0, kDebugScene, "displayMessage: %s", str.c_str());
	message = str;
	messagePos = (pos.x | pos.y) ? pos : messagePosition(str, position);
	messageColor = color;
	messageTimer = messageDuration(message);
}

void Scene::clear() {
	clearMessage();
	events.clear();
	currentEvent.clear();
	for (int i = 0; i < 4; ++i) {
		animation[i].free();
		customAnimation[i].free();
	}
	callback = 0;
	callbackTimer = 0;
}

void Scene::clearMessage() {
	message.clear();
	messageTimer = 0;
	messageColor = textColorMark;
	messageFirstFrame = 0;
	messageLastFrame = 0;
	messageAnimation = NULL;
}

} // End of namespace TeenAgent
