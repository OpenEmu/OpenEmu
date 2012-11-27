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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "lastexpress/data/scene.h"

#include "lastexpress/data/background.h"

#include "lastexpress/helpers.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"

#include "common/stream.h"

namespace LastExpress {

SceneHotspot::~SceneHotspot() {
	for (uint i = 0; i < _coords.size(); i++)
		SAFE_DELETE(_coords[i]);

	_coords.clear();
}

SceneHotspot *SceneHotspot::load(Common::SeekableReadStream *stream) {
	SceneHotspot *hs = new SceneHotspot();

	// Rect
	hs->rect.left = (int16)stream->readUint16LE();
	hs->rect.right = (int16)stream->readUint16LE();
	hs->rect.top = (int16)stream->readUint16LE();
	hs->rect.bottom = (int16)stream->readUint16LE();

	hs->coordsOffset = stream->readUint32LE();
	hs->scene = (SceneIndex)stream->readUint16LE();
	hs->location = stream->readByte();
	hs->action = (Action)stream->readByte();
	hs->param1 = stream->readByte();
	hs->param2 = stream->readByte();
	hs->param3 = stream->readByte();
	hs->cursor = stream->readByte();
	hs->next = stream->readUint32LE();

	debugC(10, kLastExpressDebugScenes, "\thotspot: scene=%d location=%02d action=%d param1=%02d param2=%02d param3=%02d cursor=%02d rect=(%d, %d)x(%d, %d)",
	                                   hs->scene, hs->location, hs->action, hs->param1, hs->param2, hs->param3, hs->cursor, hs->rect.left, hs->rect.top, hs->rect.right, hs->rect.bottom);
	debugC(10, kLastExpressDebugScenes, "\t         coords=%d next=%d ", hs->coordsOffset, hs->next);

	// Read all coords data
	uint32 offset = hs->coordsOffset;
	while (offset != 0) {

		SceneCoord *sceneCoord = new SceneCoord;

		stream->seek(offset, SEEK_SET);

		sceneCoord->field_0 = stream->readSint32LE();
		sceneCoord->field_4 = stream->readSint32LE();
		sceneCoord->field_8 = stream->readByte();
		sceneCoord->next = stream->readUint32LE();

		hs->_coords.push_back(sceneCoord);

		offset = sceneCoord->next;
	}

	return hs;
}

Common::String SceneHotspot::toString() const {
	Common::String output = "";

	output += Common::String::format("    hotspot: scene=%d location=%02d action=%d param1=%02d param2=%02d param3=%02d cursor=%02d rect=(%d, %d)x(%d, %d)",
	                                   scene, location, action, param1, param2, param3, cursor, rect.left, rect.top, rect.right, rect.bottom);

	return output;
}

bool SceneHotspot::isInside(const Common::Point &point) {

	bool contains = rect.contains(point);

	if (_coords.empty() || !contains)
		return contains;

	// Checks extended coordinates
	for (uint i = 0; i < _coords.size(); i++) {

		SceneCoord *sCoord = _coords[i];

		bool cont;
		if (sCoord->field_8)
			cont = (sCoord->field_4 + point.x * sCoord->field_0 + 1000 * point.y) >= 0;
		else
			cont = (sCoord->field_4 + point.x * sCoord->field_0 + 1000 * point.y) <= 0;

		if (!cont)
			return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Scene
Scene::~Scene() {
	// Free the hotspots
	for (uint i = 0; i < _hotspots.size(); i++)
		delete _hotspots[i];
}

Scene *Scene::load(Common::SeekableReadStream *stream) {
	Scene *scene = new Scene();

	stream->read(&scene->_name, sizeof(scene->_name));
	scene->_sig = stream->readByte();
	scene->entityPosition = (EntityPosition)stream->readUint16LE();
	scene->location = (Location)stream->readUint16LE();
	scene->car = (CarIndex)stream->readUint16LE();
	scene->position = stream->readByte();
	scene->type = (Type)stream->readByte();
	scene->param1 = stream->readByte();
	scene->param2 = stream->readByte();
	scene->param3 = stream->readByte();
	scene->_hotspot = stream->readUint32LE();

	return scene;
}

void Scene::loadHotspots(Common::SeekableReadStream *stream) {
	if (!_hotspots.empty())
		return;

	debugC(10, kLastExpressDebugScenes, "Scene:  name=%s, sig=%02d, entityPosition=%d, location=%d", _name, _sig, entityPosition, location);
	debugC(10, kLastExpressDebugScenes, "\tcar=%02d, position=%02d, type=%02d, param1=%02d", car, position, type, param1);
	debugC(10, kLastExpressDebugScenes, "\tparam2=%02d, param3=%02d, hotspot=%d\n", param2, param3, _hotspot);

	// Read all hotspots
	if (_hotspot != 0) {
		stream->seek((int32)_hotspot, SEEK_SET);
		SceneHotspot *hotspot = SceneHotspot::load(stream);
		while (hotspot) {
			_hotspots.push_back(hotspot);

			if (hotspot->next == 0)
				break;

			stream->seek((int32)hotspot->next, SEEK_SET);
			hotspot = SceneHotspot::load(stream);
		}
	}
}

bool Scene::checkHotSpot(const Common::Point &coord, SceneHotspot **hotspot) {
	bool found = false;
	int _location = 0;

	for (uint i = 0; i < _hotspots.size(); i++) {
		if (_hotspots[i]->isInside(coord)) {
			if (_location <= _hotspots[i]->location) {
				_location = _hotspots[i]->location;
				*hotspot = _hotspots[i];
				found = true;
			}
		}
	}

	return found;
}

SceneHotspot *Scene::getHotspot(uint index) {
	if (_hotspots.empty())
		error("[Scene::getHotspot] Scene does not have any hotspots");

	if (index >= _hotspots.size())
		error("[Scene::getHotspot] Invalid index (was: %d, max: %d)", index, _hotspots.size() - 1);

	return _hotspots[index];
}

Common::Rect Scene::draw(Graphics::Surface *surface) {
	// Safety checks
	Common::Rect rect;

	Common::String sceneName(_name);
	sceneName.trim();
	if (sceneName.empty())
		error("[Scene::draw] This scene is not a valid drawing scene");

	// Load background
	Background *background = ((LastExpressEngine *)g_engine)->getResourceManager()->loadBackground(sceneName);
	if (background) {
		rect = background->draw(surface);
		delete background;
	}

	return rect;
}

Common::String Scene::toString() {
	Common::String output = "";

	output += Common::String::format("Scene:  name=%s, sig=%02d, entityPosition=%d, location=%d\n", _name, _sig, entityPosition, location);
	output += Common::String::format("        car=%02d, position=%02d, type=%02d, param1=%02d\n", car, position, type, param1);
	output += Common::String::format("        param2=%02d, param3=%02d, hotspot=%d\n", param2, param3, _hotspot);

	// Hotspots
	if (_hotspots.size() != 0) {
		output += "\nHotspots:\n";
		for (uint i = 0; i < _hotspots.size(); i++)
			output += _hotspots[i]->toString() + "\n";
	}

	return output;
}

//////////////////////////////////////////////////////////////////////////
// SceneLoader
SceneLoader::SceneLoader() : _stream(NULL) {}

SceneLoader::~SceneLoader() {
	clear();
}

void SceneLoader::clear() {
	// Remove all scenes
	for (uint i = 0; i < _scenes.size(); i++)
		delete _scenes[i];

	_scenes.clear();

	delete _stream;
}

bool SceneLoader::load(Common::SeekableReadStream *stream) {
	if (!stream)
		return false;

	clear();

	_stream = stream;

	// Read the default scene to get the total number of scenes
	Scene *header = Scene::load(_stream);
	if (!header)
		error("[SceneLoader::load] Invalid data file");

	debugC(2, kLastExpressDebugScenes, "   found %d entries", header->entityPosition); /* Header entityPosition is the scene count */

	if (header->entityPosition > 2500) {
		delete header;

		return false;
	}

	_scenes.push_back(header);

	// Read all the chunks
	for (uint i = 0; i < (uint)header->entityPosition; ++i) {
		Scene *scene = Scene::load(_stream);
		if (!scene)
			break;

		_scenes.push_back(scene);
	}

	return true;
}

Scene *SceneLoader::get(SceneIndex index) {
	if (_scenes.empty())
		return NULL;

	if (index > _scenes.size())
		return NULL;

	// Load the hotspots if needed
	_scenes[(uint)index]->loadHotspots(_stream);

	return _scenes[(uint)index];
}

} // End of namespace LastExpress
