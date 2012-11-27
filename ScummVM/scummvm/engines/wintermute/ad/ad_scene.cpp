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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/ad/ad_scene.h"
#include "engines/wintermute/ad/ad_actor.h"
#include "engines/wintermute/ad/ad_entity.h"
#include "engines/wintermute/ad/ad_game.h"
#include "engines/wintermute/ad/ad_layer.h"
#include "engines/wintermute/ad/ad_node_state.h"
#include "engines/wintermute/ad/ad_object.h"
#include "engines/wintermute/ad/ad_path.h"
#include "engines/wintermute/ad/ad_path_point.h"
#include "engines/wintermute/ad/ad_rot_level.h"
#include "engines/wintermute/ad/ad_scale_level.h"
#include "engines/wintermute/ad/ad_scene_node.h"
#include "engines/wintermute/ad/ad_scene_state.h"
#include "engines/wintermute/ad/ad_sentence.h"
#include "engines/wintermute/ad/ad_waypoint_group.h"
#include "engines/wintermute/base/base_dynamic_buffer.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/font/base_font.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_object.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_point.h"
#include "engines/wintermute/base/base_region.h"
#include "engines/wintermute/base/base_scriptable.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/base/base_viewport.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/ui/ui_window.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/wintermute.h"
#include <limits.h>

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdScene, false)

//////////////////////////////////////////////////////////////////////////
AdScene::AdScene(BaseGame *inGame) : BaseObject(inGame) {
	_pfTarget = new BasePoint;
	setDefaults();
}


//////////////////////////////////////////////////////////////////////////
AdScene::~AdScene() {
	cleanup();
	_gameRef->unregisterObject(_fader);
	delete _pfTarget;
	_pfTarget = NULL;
}


//////////////////////////////////////////////////////////////////////////
void AdScene::setDefaults() {
	_initialized = false;
	_pfReady = true;
	_pfTargetPath = NULL;
	_pfRequester = NULL;
	_mainLayer = NULL;

	_pfPointsNum = 0;
	_persistentState = false;
	_persistentStateSprites = true;

	_autoScroll = true;
	_offsetLeft = _offsetTop = 0;
	_targetOffsetLeft = _targetOffsetTop = 0;

	_lastTimeH = _lastTimeV = 0;
	_scrollTimeH = _scrollTimeV = 10;
	_scrollPixelsH = _scrollPixelsV = 1;

	_pfMaxTime = 15;

	_paralaxScrolling = true;

	// editor settings
	_editorMarginH = _editorMarginV = 100;

	_editorColFrame        = 0xE0888888;
	_editorColEntity       = 0xFF008000;
	_editorColRegion       = 0xFF0000FF;
	_editorColBlocked      = 0xFF800080;
	_editorColWaypoints    = 0xFF0000FF;
	_editorColEntitySel    = 0xFFFF0000;
	_editorColRegionSel    = 0xFFFF0000;
	_editorColBlockedSel   = 0xFFFF0000;
	_editorColWaypointsSel = 0xFFFF0000;
	_editorColScale        = 0xFF00FF00;
	_editorColDecor        = 0xFF00FFFF;
	_editorColDecorSel     = 0xFFFF0000;

	_editorShowRegions  = true;
	_editorShowBlocked  = true;
	_editorShowDecor    = true;
	_editorShowEntities = true;
	_editorShowScale    = true;

	_shieldWindow = NULL;

	_fader = new BaseFader(_gameRef);
	_gameRef->registerObject(_fader);

	_viewport = NULL;
}


//////////////////////////////////////////////////////////////////////////
void AdScene::cleanup() {
	BaseObject::cleanup();

	_mainLayer = NULL; // reference only

	delete _shieldWindow;
	_shieldWindow = NULL;

	_gameRef->unregisterObject(_fader);
	_fader = NULL;

	for (uint32 i = 0; i < _layers.size(); i++) {
		_gameRef->unregisterObject(_layers[i]);
	}
	_layers.clear();


	for (uint32 i = 0; i < _waypointGroups.size(); i++) {
		_gameRef->unregisterObject(_waypointGroups[i]);
	}
	_waypointGroups.clear();

	for (uint32 i = 0; i < _scaleLevels.size(); i++) {
		_gameRef->unregisterObject(_scaleLevels[i]);
	}
	_scaleLevels.clear();

	for (uint32 i = 0; i < _rotLevels.size(); i++) {
		_gameRef->unregisterObject(_rotLevels[i]);
	}
	_rotLevels.clear();


	for (uint32 i = 0; i < _pfPath.size(); i++) {
		delete _pfPath[i];
	}
	_pfPath.clear();
	_pfPointsNum = 0;

	for (uint32 i = 0; i < _objects.size(); i++) {
		_gameRef->unregisterObject(_objects[i]);
	}
	_objects.clear();

	delete _viewport;
	_viewport = NULL;

	setDefaults();
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::getPath(BasePoint source, BasePoint target, AdPath *path, BaseObject *requester) {
	if (!_pfReady) {
		return false;
	} else {
		_pfReady = false;
		*_pfTarget = target;
		_pfTargetPath = path;
		_pfRequester = requester;

		_pfTargetPath->reset();
		_pfTargetPath->setReady(false);

		// prepare working path
		pfPointsStart();

		// first point
		//_pfPath.add(new AdPathPoint(source.x, source.y, 0));

		// if we're one pixel stuck, get unstuck
		int startX = source.x;
		int startY = source.y;
		int bestDistance = 1000;
		if (isBlockedAt(startX, startY, true, requester)) {
			int tolerance = 2;
			for (int xxx = startX - tolerance; xxx <= startX + tolerance; xxx++) {
				for (int yyy = startY - tolerance; yyy <= startY + tolerance; yyy++) {
					if (isWalkableAt(xxx, yyy, true, requester)) {
						int distance = abs(xxx - source.x) + abs(yyy - source.y);
						if (distance < bestDistance) {
							startX = xxx;
							startY = yyy;

							bestDistance = distance;
						}
					}
				}
			}
		}

		pfPointsAdd(startX, startY, 0);

		//CorrectTargetPoint(&target.x, &target.y);

		// last point
		//_pfPath.add(new AdPathPoint(target.x, target.y, INT_MAX));
		pfPointsAdd(target.x, target.y, INT_MAX);

		// active waypoints
		for (uint32 i = 0; i < _waypointGroups.size(); i++) {
			if (_waypointGroups[i]->_active) {
				pfAddWaypointGroup(_waypointGroups[i], requester);
			}
		}


		// free waypoints
		for (uint32 i = 0; i < _objects.size(); i++) {
			if (_objects[i]->_active && _objects[i] != requester && _objects[i]->_currentWptGroup) {
				pfAddWaypointGroup(_objects[i]->_currentWptGroup, requester);
			}
		}
		AdGame *adGame = (AdGame *)_gameRef;
		for (uint32 i = 0; i < adGame->_objects.size(); i++) {
			if (adGame->_objects[i]->_active && adGame->_objects[i] != requester && adGame->_objects[i]->_currentWptGroup) {
				pfAddWaypointGroup(adGame->_objects[i]->_currentWptGroup, requester);
			}
		}

		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
void AdScene::pfAddWaypointGroup(AdWaypointGroup *wpt, BaseObject *requester) {
	if (!wpt->_active) {
		return;
	}

	for (uint32 i = 0; i < wpt->_points.size(); i++) {
		if (isBlockedAt(wpt->_points[i]->x, wpt->_points[i]->y, true, requester)) {
			continue;
		}

		//_pfPath.add(new AdPathPoint(Wpt->_points[i]->x, Wpt->_points[i]->y, INT_MAX));
		pfPointsAdd(wpt->_points[i]->x, wpt->_points[i]->y, INT_MAX);
	}
}


//////////////////////////////////////////////////////////////////////////
float AdScene::getZoomAt(int x, int y) {
	float ret = 100;

	bool found = false;
	if (_mainLayer) {
		for (int i = _mainLayer->_nodes.size() - 1; i >= 0; i--) {
			AdSceneNode *node = _mainLayer->_nodes[i];
			if (node->_type == OBJECT_REGION && node->_region->_active && !node->_region->_blocked && node->_region->pointInRegion(x, y)) {
				if (node->_region->_zoom != 0) {
					ret = node->_region->_zoom;
					found = true;
					break;
				}
			}
		}
	}
	if (!found) {
		ret = getScaleAt(y);
	}

	return ret;
}


//////////////////////////////////////////////////////////////////////////
uint32 AdScene::getAlphaAt(int x, int y, bool colorCheck) {
	if (!_gameRef->_debugDebugMode) {
		colorCheck = false;
	}

	uint32 ret;
	if (colorCheck) {
		ret = 0xFFFF0000;
	} else {
		ret = 0xFFFFFFFF;
	}

	if (_mainLayer) {
		for (int i = _mainLayer->_nodes.size() - 1; i >= 0; i--) {
			AdSceneNode *node = _mainLayer->_nodes[i];
			if (node->_type == OBJECT_REGION && node->_region->_active && (colorCheck || !node->_region->_blocked) && node->_region->pointInRegion(x, y)) {
				if (!node->_region->_blocked) {
					ret = node->_region->_alpha;
				}
				break;
			}
		}
	}
	return ret;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::isBlockedAt(int x, int y, bool checkFreeObjects, BaseObject *requester) {
	bool ret = true;

	if (checkFreeObjects) {
		for (uint32 i = 0; i < _objects.size(); i++) {
			if (_objects[i]->_active && _objects[i] != requester && _objects[i]->_currentBlockRegion) {
				if (_objects[i]->_currentBlockRegion->pointInRegion(x, y)) {
					return true;
				}
			}
		}
		AdGame *adGame = (AdGame *)_gameRef;
		for (uint32 i = 0; i < adGame->_objects.size(); i++) {
			if (adGame->_objects[i]->_active && adGame->_objects[i] != requester && adGame->_objects[i]->_currentBlockRegion) {
				if (adGame->_objects[i]->_currentBlockRegion->pointInRegion(x, y)) {
					return true;
				}
			}
		}
	}


	if (_mainLayer) {
		for (uint32 i = 0; i < _mainLayer->_nodes.size(); i++) {
			AdSceneNode *node = _mainLayer->_nodes[i];
			/*
			if (Node->_type == OBJECT_REGION && Node->_region->_active && Node->_region->_blocked && Node->_region->PointInRegion(X, Y))
			{
			    ret = true;
			    break;
			}
			*/
			if (node->_type == OBJECT_REGION && node->_region->_active && !node->_region->_decoration && node->_region->pointInRegion(x, y)) {
				if (node->_region->_blocked) {
					ret = true;
					break;
				} else {
					ret = false;
				}
			}
		}
	}
	return ret;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::isWalkableAt(int x, int y, bool checkFreeObjects, BaseObject *requester) {
	bool ret = false;

	if (checkFreeObjects) {
		for (uint32 i = 0; i < _objects.size(); i++) {
			if (_objects[i]->_active && _objects[i] != requester && _objects[i]->_currentBlockRegion) {
				if (_objects[i]->_currentBlockRegion->pointInRegion(x, y)) {
					return false;
				}
			}
		}
		AdGame *adGame = (AdGame *)_gameRef;
		for (uint32 i = 0; i < adGame->_objects.size(); i++) {
			if (adGame->_objects[i]->_active && adGame->_objects[i] != requester && adGame->_objects[i]->_currentBlockRegion) {
				if (adGame->_objects[i]->_currentBlockRegion->pointInRegion(x, y)) {
					return false;
				}
			}
		}
	}


	if (_mainLayer) {
		for (uint32 i = 0; i < _mainLayer->_nodes.size(); i++) {
			AdSceneNode *node = _mainLayer->_nodes[i];
			if (node->_type == OBJECT_REGION && node->_region->_active && !node->_region->_decoration && node->_region->pointInRegion(x, y)) {
				if (node->_region->_blocked) {
					ret = false;
					break;
				} else {
					ret = true;
				}
			}
		}
	}
	return ret;
}


//////////////////////////////////////////////////////////////////////////
int AdScene::getPointsDist(BasePoint p1, BasePoint p2, BaseObject *requester) {
	double xStep, yStep, x, y;
	int xLength, yLength, xCount, yCount;
	int x1, y1, x2, y2;

	x1 = p1.x;
	y1 = p1.y;
	x2 = p2.x;
	y2 = p2.y;

	xLength = abs(x2 - x1);
	yLength = abs(y2 - y1);

	if (xLength > yLength) {
		if (x1 > x2) {
			BaseUtils::swap(&x1, &x2);
			BaseUtils::swap(&y1, &y2);
		}

		yStep = (double)(y2 - y1) / (double)(x2 - x1);
		y = y1;

		for (xCount = x1; xCount < x2; xCount++) {
			if (isBlockedAt(xCount, (int)y, true, requester)) {
				return -1;
			}
			y += yStep;
		}
	} else {
		if (y1 > y2) {
			BaseUtils::swap(&x1, &x2);
			BaseUtils::swap(&y1, &y2);
		}

		xStep = (double)(x2 - x1) / (double)(y2 - y1);
		x = x1;

		for (yCount = y1; yCount < y2; yCount++) {
			if (isBlockedAt((int)x, yCount, true, requester)) {
				return -1;
			}
			x += xStep;
		}
	}
	return MAX(xLength, yLength);
}


//////////////////////////////////////////////////////////////////////////
void AdScene::pathFinderStep() {
	int i;
	// get lowest unmarked
	int lowestDist = INT_MAX;
	AdPathPoint *lowestPt = NULL;

	for (i = 0; i < _pfPointsNum; i++)
		if (!_pfPath[i]->_marked && _pfPath[i]->_distance < lowestDist) {
			lowestDist = _pfPath[i]->_distance;
			lowestPt = _pfPath[i];
		}

	if (lowestPt == NULL) { // no path -> terminate PathFinder
		_pfReady = true;
		_pfTargetPath->setReady(true);
		return;
	}

	lowestPt->_marked = true;

	// target point marked, generate path and terminate
	if (lowestPt->x == _pfTarget->x && lowestPt->y == _pfTarget->y) {
		while (lowestPt != NULL) {
			_pfTargetPath->_points.insert_at(0, new BasePoint(lowestPt->x, lowestPt->y));
			lowestPt = lowestPt->_origin;
		}

		_pfReady = true;
		_pfTargetPath->setReady(true);
		return;
	}

	// otherwise keep on searching
	for (i = 0; i < _pfPointsNum; i++)
		if (!_pfPath[i]->_marked) {
			int j = getPointsDist(*lowestPt, *_pfPath[i], _pfRequester);
			if (j != -1 && lowestPt->_distance + j < _pfPath[i]->_distance) {
				_pfPath[i]->_distance = lowestPt->_distance + j;
				_pfPath[i]->_origin = lowestPt;
			}
		}
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::initLoop() {
#ifdef _DEBUGxxxx
	int nu_steps = 0;
	uint32 start = _gameRef->_currentTime;
	while (!_pfReady && g_system->getMillis() - start <= _pfMaxTime) {
		PathFinderStep();
		nu_steps++;
	}
	if (nu_steps > 0) {
		_gameRef->LOG(0, "STAT: PathFinder iterations in one loop: %d (%s)  _pfMaxTime=%d", nu_steps, _pfReady ? "finished" : "not yet done", _pfMaxTime);
	}
#else
	uint32 start = _gameRef->_currentTime;
	while (!_pfReady && g_system->getMillis() - start <= _pfMaxTime) {
		pathFinderStep();
	}
#endif

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::loadFile(const char *filename) {
	byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == NULL) {
		_gameRef->LOG(0, "AdScene::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) {
		_gameRef->LOG(0, "Error parsing SCENE file '%s'", filename);
	}

	setFilename(filename);

	delete[] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(SCENE)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(NAME)
TOKEN_DEF(LAYER)
TOKEN_DEF(WAYPOINTS)
TOKEN_DEF(EVENTS)
TOKEN_DEF(CURSOR)
TOKEN_DEF(CAMERA)
TOKEN_DEF(ENTITY)
TOKEN_DEF(SCALE_LEVEL)
TOKEN_DEF(ROTATION_LEVEL)
TOKEN_DEF(EDITOR_MARGIN_H)
TOKEN_DEF(EDITOR_MARGIN_V)
TOKEN_DEF(EDITOR_COLOR_FRAME)
TOKEN_DEF(EDITOR_COLOR_ENTITY_SEL)
TOKEN_DEF(EDITOR_COLOR_REGION_SEL)
TOKEN_DEF(EDITOR_COLOR_DECORATION_SEL)
TOKEN_DEF(EDITOR_COLOR_BLOCKED_SEL)
TOKEN_DEF(EDITOR_COLOR_WAYPOINTS_SEL)
TOKEN_DEF(EDITOR_COLOR_REGION)
TOKEN_DEF(EDITOR_COLOR_DECORATION)
TOKEN_DEF(EDITOR_COLOR_BLOCKED)
TOKEN_DEF(EDITOR_COLOR_ENTITY)
TOKEN_DEF(EDITOR_COLOR_WAYPOINTS)
TOKEN_DEF(EDITOR_COLOR_SCALE)
TOKEN_DEF(EDITOR_SHOW_REGIONS)
TOKEN_DEF(EDITOR_SHOW_BLOCKED)
TOKEN_DEF(EDITOR_SHOW_DECORATION)
TOKEN_DEF(EDITOR_SHOW_ENTITIES)
TOKEN_DEF(EDITOR_SHOW_SCALE)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(CAPTION)
TOKEN_DEF(PROPERTY)
TOKEN_DEF(VIEWPORT)
TOKEN_DEF(PERSISTENT_STATE_SPRITES)
TOKEN_DEF(PERSISTENT_STATE)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool AdScene::loadBuffer(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(SCENE)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(LAYER)
	TOKEN_TABLE(WAYPOINTS)
	TOKEN_TABLE(EVENTS)
	TOKEN_TABLE(CURSOR)
	TOKEN_TABLE(CAMERA)
	TOKEN_TABLE(ENTITY)
	TOKEN_TABLE(SCALE_LEVEL)
	TOKEN_TABLE(ROTATION_LEVEL)
	TOKEN_TABLE(EDITOR_MARGIN_H)
	TOKEN_TABLE(EDITOR_MARGIN_V)
	TOKEN_TABLE(EDITOR_COLOR_FRAME)
	TOKEN_TABLE(EDITOR_COLOR_ENTITY_SEL)
	TOKEN_TABLE(EDITOR_COLOR_REGION_SEL)
	TOKEN_TABLE(EDITOR_COLOR_DECORATION_SEL)
	TOKEN_TABLE(EDITOR_COLOR_BLOCKED_SEL)
	TOKEN_TABLE(EDITOR_COLOR_WAYPOINTS_SEL)
	TOKEN_TABLE(EDITOR_COLOR_REGION)
	TOKEN_TABLE(EDITOR_COLOR_DECORATION)
	TOKEN_TABLE(EDITOR_COLOR_BLOCKED)
	TOKEN_TABLE(EDITOR_COLOR_ENTITY)
	TOKEN_TABLE(EDITOR_COLOR_WAYPOINTS)
	TOKEN_TABLE(EDITOR_COLOR_SCALE)
	TOKEN_TABLE(EDITOR_SHOW_REGIONS)
	TOKEN_TABLE(EDITOR_SHOW_DECORATION)
	TOKEN_TABLE(EDITOR_SHOW_BLOCKED)
	TOKEN_TABLE(EDITOR_SHOW_ENTITIES)
	TOKEN_TABLE(EDITOR_SHOW_SCALE)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(PROPERTY)
	TOKEN_TABLE(VIEWPORT)
	TOKEN_TABLE(PERSISTENT_STATE_SPRITES)
	TOKEN_TABLE(PERSISTENT_STATE)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	cleanup();

	byte *params;
	int cmd;
	BaseParser parser;

	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_SCENE) {
			_gameRef->LOG(0, "'SCENE' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	int ar, ag, ab, aa;
	char camera[MAX_PATH_LENGTH] = "";
	/* float waypointHeight = -1.0f; */

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (DID_FAIL(loadFile((char *)params))) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_CAPTION:
			setCaption((char *)params);
			break;

		case TOKEN_LAYER: {
			AdLayer *layer = new AdLayer(_gameRef);
			if (!layer || DID_FAIL(layer->loadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				delete layer;
				layer = NULL;
			} else {
				_gameRef->registerObject(layer);
				_layers.add(layer);
				if (layer->_main) {
					_mainLayer = layer;
					_width = layer->_width;
					_height = layer->_height;
				}
			}
		}
		break;

		case TOKEN_WAYPOINTS: {
			AdWaypointGroup *wpt = new AdWaypointGroup(_gameRef);
			if (!wpt || DID_FAIL(wpt->loadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				delete wpt;
				wpt = NULL;
			} else {
				_gameRef->registerObject(wpt);
				_waypointGroups.add(wpt);
			}
		}
		break;

		case TOKEN_SCALE_LEVEL: {
			AdScaleLevel *sl = new AdScaleLevel(_gameRef);
			if (!sl || DID_FAIL(sl->loadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				delete sl;
				sl = NULL;
			} else {
				_gameRef->registerObject(sl);
				_scaleLevels.add(sl);
			}
		}
		break;

		case TOKEN_ROTATION_LEVEL: {
			AdRotLevel *rl = new AdRotLevel(_gameRef);
			if (!rl || DID_FAIL(rl->loadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				delete rl;
				rl = NULL;
			} else {
				_gameRef->registerObject(rl);
				_rotLevels.add(rl);
			}
		}
		break;

		case TOKEN_ENTITY: {
			AdEntity *entity = new AdEntity(_gameRef);
			if (!entity || DID_FAIL(entity->loadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				delete entity;
				entity = NULL;
			} else {
				addObject(entity);
			}
		}
		break;

		case TOKEN_CURSOR:
			delete _cursor;
			_cursor = new BaseSprite(_gameRef);
			if (!_cursor || DID_FAIL(_cursor->loadFile((char *)params))) {
				delete _cursor;
				_cursor = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_CAMERA:
			strcpy(camera, (char *)params);
			break;

		case TOKEN_EDITOR_MARGIN_H:
			parser.scanStr((char *)params, "%d", &_editorMarginH);
			break;

		case TOKEN_EDITOR_MARGIN_V:
			parser.scanStr((char *)params, "%d", &_editorMarginV);
			break;

		case TOKEN_EDITOR_COLOR_FRAME:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColFrame = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_ENTITY:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColEntity = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_ENTITY_SEL:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColEntitySel = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_REGION_SEL:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColRegionSel = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_DECORATION_SEL:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColDecorSel = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_BLOCKED_SEL:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColBlockedSel = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_WAYPOINTS_SEL:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColWaypointsSel = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_REGION:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColRegion = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_DECORATION:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColDecor = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_BLOCKED:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColBlocked = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_WAYPOINTS:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColWaypoints = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_COLOR_SCALE:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &ar, &ag, &ab, &aa);
			_editorColScale = BYTETORGBA(ar, ag, ab, aa);
			break;

		case TOKEN_EDITOR_SHOW_REGIONS:
			parser.scanStr((char *)params, "%b", &_editorShowRegions);
			break;

		case TOKEN_EDITOR_SHOW_BLOCKED:
			parser.scanStr((char *)params, "%b", &_editorShowBlocked);
			break;

		case TOKEN_EDITOR_SHOW_DECORATION:
			parser.scanStr((char *)params, "%b", &_editorShowDecor);
			break;

		case TOKEN_EDITOR_SHOW_ENTITIES:
			parser.scanStr((char *)params, "%b", &_editorShowEntities);
			break;

		case TOKEN_EDITOR_SHOW_SCALE:
			parser.scanStr((char *)params, "%b", &_editorShowScale);
			break;

		case TOKEN_SCRIPT:
			addScript((char *)params);
			break;

		case TOKEN_PROPERTY:
			parseProperty(params, false);
			break;

		case TOKEN_VIEWPORT: {
			Rect32 rc;
			parser.scanStr((char *)params, "%d,%d,%d,%d", &rc.left, &rc.top, &rc.right, &rc.bottom);
			if (!_viewport) {
				_viewport = new BaseViewport(_gameRef);
			}
			if (_viewport) {
				_viewport->setRect(rc.left, rc.top, rc.right, rc.bottom, true);
			}
		}

		case TOKEN_PERSISTENT_STATE:
			parser.scanStr((char *)params, "%b", &_persistentState);
			break;

		case TOKEN_PERSISTENT_STATE_SPRITES:
			parser.scanStr((char *)params, "%b", &_persistentStateSprites);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;

		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in SCENE definition");
		return STATUS_FAILED;
	}

	if (_mainLayer == NULL) {
		_gameRef->LOG(0, "Warning: scene '%s' has no main layer.", getFilename());
	}


	sortScaleLevels();
	sortRotLevels();

	_initialized = true;


	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdScene::traverseNodes(bool doUpdate) {
	if (!_initialized) {
		return STATUS_OK;
	}

	AdGame *adGame = (AdGame *)_gameRef;


	//////////////////////////////////////////////////////////////////////////
	// prepare viewport
	bool popViewport = false;
	if (_viewport && !_gameRef->_editorMode) {
		_gameRef->pushViewport(_viewport);
		popViewport = true;
	} else if (adGame->_sceneViewport && !_gameRef->_editorMode) {
		_gameRef->pushViewport(adGame->_sceneViewport);
		popViewport = true;
	}


	//////////////////////////////////////////////////////////////////////////
	// *** adjust scroll offset
	if (doUpdate) {
		/*
		if (_autoScroll && _gameRef->_mainObject != NULL)
		{
		    ScrollToObject(_gameRef->_mainObject);
		}
		*/

		if (_autoScroll) {
			// adjust horizontal scroll
			if (_gameRef->_timer - _lastTimeH >= _scrollTimeH) {
				_lastTimeH = _gameRef->_timer;
				if (_offsetLeft < _targetOffsetLeft) {
					_offsetLeft += _scrollPixelsH;
					_offsetLeft = MIN(_offsetLeft, _targetOffsetLeft);
				} else if (_offsetLeft > _targetOffsetLeft) {
					_offsetLeft -= _scrollPixelsH;
					_offsetLeft = MAX(_offsetLeft, _targetOffsetLeft);
				}
			}

			// adjust vertical scroll
			if (_gameRef->_timer - _lastTimeV >= _scrollTimeV) {
				_lastTimeV = _gameRef->_timer;
				if (_offsetTop < _targetOffsetTop) {
					_offsetTop += _scrollPixelsV;
					_offsetTop = MIN(_offsetTop, _targetOffsetTop);
				} else if (_offsetTop > _targetOffsetTop) {
					_offsetTop -= _scrollPixelsV;
					_offsetTop = MAX(_offsetTop, _targetOffsetTop);
				}
			}

			if (_offsetTop == _targetOffsetTop && _offsetLeft == _targetOffsetLeft) {
				_ready = true;
			}
		} else {
			_ready = true;    // not scrolling, i.e. always ready
		}
	}




	//////////////////////////////////////////////////////////////////////////
	int viewportWidth, viewportHeight;
	getViewportSize(&viewportWidth, &viewportHeight);

	int viewportX, viewportY;
	getViewportOffset(&viewportX, &viewportY);

	int scrollableX = _width  - viewportWidth;
	int scrollableY = _height - viewportHeight;

	double widthRatio  = scrollableX <= 0 ? 0 : ((double)(_offsetLeft) / (double)scrollableX);
	double heightRatio = scrollableY <= 0 ? 0 : ((double)(_offsetTop)  / (double)scrollableY);

	int origX, origY;
	_gameRef->getOffset(&origX, &origY);



	//////////////////////////////////////////////////////////////////////////
	// *** display/update everything
	_gameRef->_renderer->setup2D();

	// for each layer
	/* int mainOffsetX = 0; */
	/* int mainOffsetY = 0; */

	for (uint32 j = 0; j < _layers.size(); j++) {
		if (!_layers[j]->_active) {
			continue;
		}

		// make layer exclusive
		if (!doUpdate) {
			if (_layers[j]->_closeUp && !_gameRef->_editorMode) {
				if (!_shieldWindow) {
					_shieldWindow = new UIWindow(_gameRef);
				}
				if (_shieldWindow) {
					_shieldWindow->_posX = _shieldWindow->_posY = 0;
					_shieldWindow->_width = _gameRef->_renderer->_width;
					_shieldWindow->_height = _gameRef->_renderer->_height;
					_shieldWindow->display();
				}
			}
		}

		if (_paralaxScrolling) {
			int offsetX = (int)(widthRatio  * (_layers[j]->_width  - viewportWidth) - viewportX);
			int offsetY = (int)(heightRatio * (_layers[j]->_height - viewportHeight) - viewportY);
			_gameRef->setOffset(offsetX, offsetY);

			_gameRef->_offsetPercentX = (float)offsetX / ((float)_layers[j]->_width - viewportWidth) * 100.0f;
			_gameRef->_offsetPercentY = (float)offsetY / ((float)_layers[j]->_height - viewportHeight) * 100.0f;

			//_gameRef->QuickMessageForm("%d %f", OffsetX+ViewportX, _gameRef->_offsetPercentX);
		} else {
			_gameRef->setOffset(_offsetLeft - viewportX, _offsetTop - viewportY);

			_gameRef->_offsetPercentX = (float)(_offsetLeft - viewportX) / ((float)_layers[j]->_width - viewportWidth) * 100.0f;
			_gameRef->_offsetPercentY = (float)(_offsetTop - viewportY) / ((float)_layers[j]->_height - viewportHeight) * 100.0f;
		}


		// for each node
		for (uint32 k = 0; k < _layers[j]->_nodes.size(); k++) {
			AdSceneNode *node = _layers[j]->_nodes[k];
			switch (node->_type) {
			case OBJECT_ENTITY:
				if (node->_entity->_active && (_gameRef->_editorMode || !node->_entity->_editorOnly)) {
					_gameRef->_renderer->setup2D();

					if (doUpdate) {
						node->_entity->update();
					} else {
						node->_entity->display();
					}
				}
				break;

			case OBJECT_REGION: {
				if (node->_region->_blocked) {
					break;
				}
				if (node->_region->_decoration) {
					break;
				}

				if (!doUpdate) {
					displayRegionContent(node->_region);
				}
			}
			break;
			default:
				error("AdScene::TraverseNodes - Unhandled enum");
				break;
			} // switch
		} // each node

		// display/update all objects which are off-regions
		if (_layers[j]->_main) {
			if (doUpdate) {
				updateFreeObjects();
			} else {
				displayRegionContent(NULL);
			}
		}
	} // each layer


	// restore state
	_gameRef->setOffset(origX, origY);
	_gameRef->_renderer->setup2D();

	// display/update fader
	if (_fader) {
		if (doUpdate) {
			_fader->update();
		} else {
			_fader->display();
		}
	}

	if (popViewport) {
		_gameRef->popViewport();
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::display() {
	return traverseNodes(false);
}

//////////////////////////////////////////////////////////////////////////
bool AdScene::updateFreeObjects() {
	AdGame *adGame = (AdGame *)_gameRef;
	// 3D-code removed
	// bool is3DSet;

	// *** update all active objects
	// is3DSet = false;
	for (uint32 i = 0; i < adGame->_objects.size(); i++) {
		if (!adGame->_objects[i]->_active) {
			continue;
		}
		// 3D-code removed
		adGame->_objects[i]->update();
		adGame->_objects[i]->_drawn = false;
	}


	for (uint32 i = 0; i < _objects.size(); i++) {
		if (!_objects[i]->_active) {
			continue;
		}

		_objects[i]->update();
		_objects[i]->_drawn = false;
	}


	if (_autoScroll && _gameRef->_mainObject != NULL) {
		scrollToObject(_gameRef->_mainObject);
	}


	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::displayRegionContent(AdRegion *region, bool display3DOnly) {
	AdGame *adGame = (AdGame *)_gameRef;
	BaseArray<AdObject *> objects;
	AdObject *obj;

	// global objects
	for (uint32 i = 0; i < adGame->_objects.size(); i++) {
		obj = adGame->_objects[i];
		if (obj->_active && !obj->_drawn && (obj->_stickRegion == region || region == NULL || (obj->_stickRegion == NULL && region->pointInRegion(obj->_posX, obj->_posY)))) {
			objects.add(obj);
		}
	}

	// scene objects
	for (uint32 i = 0; i < _objects.size(); i++) {
		obj = _objects[i];
		if (obj->_active && !obj->_editorOnly && !obj->_drawn && (obj->_stickRegion == region || region == NULL || (obj->_stickRegion == NULL && region->pointInRegion(obj->_posX, obj->_posY)))) {
			objects.add(obj);
		}
	}

	// sort by _posY
	Common::sort(objects.begin(), objects.end(), AdScene::compareObjs);

	// display them
	for (uint32 i = 0; i < objects.size(); i++) {
		obj = objects[i];

		if (display3DOnly && !obj->_is3D) {
			continue;
		}

		_gameRef->_renderer->setup2D();

		if (_gameRef->_editorMode || !obj->_editorOnly) {
			obj->display();
		}
		obj->_drawn = true;
	}


	// display design only objects
	if (!display3DOnly) {
		if (_gameRef->_editorMode && region == NULL) {
			for (uint32 i = 0; i < _objects.size(); i++) {
				if (_objects[i]->_active && _objects[i]->_editorOnly) {
					_objects[i]->display();
					_objects[i]->_drawn = true;
				}
			}
		}
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
int AdScene::compareObjs(const void *obj1, const void *obj2) {
	const AdObject *object1 = *(const AdObject *const *)obj1;
	const AdObject *object2 = *(const AdObject *const *)obj2;

	if (object1->_posY < object2->_posY) {
		return -1;
	} else if (object1->_posY > object2->_posY) {
		return 1;
	} else {
		return 0;
	}
}

//////////////////////////////////////////////////////////////////////////
bool AdScene::displayRegionContentOld(AdRegion *region) {
	AdGame *adGame = (AdGame *)_gameRef;
	AdObject *obj;

	// display all objects in region sorted by _posY
	do {
		obj = NULL;
		int minY = INT_MAX;

		// global objects
		for (uint32 i = 0; i < adGame->_objects.size(); i++) {
			if (adGame->_objects[i]->_active && !adGame->_objects[i]->_drawn && adGame->_objects[i]->_posY < minY && (adGame->_objects[i]->_stickRegion == region || region == NULL || (adGame->_objects[i]->_stickRegion == NULL && region->pointInRegion(adGame->_objects[i]->_posX, adGame->_objects[i]->_posY)))) {
				obj = adGame->_objects[i];
				minY = adGame->_objects[i]->_posY;
			}
		}

		// scene objects
		for (uint32 i = 0; i < _objects.size(); i++) {
			if (_objects[i]->_active && !_objects[i]->_editorOnly && !_objects[i]->_drawn && _objects[i]->_posY < minY && (_objects[i]->_stickRegion == region || region == NULL || (_objects[i]->_stickRegion == NULL && region->pointInRegion(_objects[i]->_posX, _objects[i]->_posY)))) {
				obj = _objects[i];
				minY = _objects[i]->_posY;
			}
		}


		if (obj != NULL) {
			_gameRef->_renderer->setup2D();

			if (_gameRef->_editorMode || !obj->_editorOnly) {
				obj->display();
			}
			obj->_drawn = true;
		}
	} while (obj != NULL);


	// design only objects
	if (_gameRef->_editorMode && region == NULL) {
		for (uint32 i = 0; i < _objects.size(); i++) {
			if (_objects[i]->_active && _objects[i]->_editorOnly) {
				_objects[i]->display();
				_objects[i]->_drawn = true;
			}
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::update() {
	return traverseNodes(true);
}

//////////////////////////////////////////////////////////////////////////
void AdScene::scrollTo(int offsetX, int offsetY) {
	int viewportWidth, viewportHeight;
	getViewportSize(&viewportWidth, &viewportHeight);

	int origOffsetLeft = _targetOffsetLeft;
	int origOffsetTop = _targetOffsetTop;

	_targetOffsetLeft = MAX(0, offsetX - viewportWidth / 2);
	_targetOffsetLeft = MIN(_targetOffsetLeft, _width - viewportWidth);

	_targetOffsetTop = MAX(0, offsetY - viewportHeight / 2);
	_targetOffsetTop = MIN(_targetOffsetTop, _height - viewportHeight);


	if (_gameRef->_mainObject && _gameRef->_mainObject->_is3D) {
		if (abs(origOffsetLeft - _targetOffsetLeft) < 5) {
			_targetOffsetLeft = origOffsetLeft;
		}
		if (abs(origOffsetTop - _targetOffsetTop) < 5) {
			_targetOffsetTop = origOffsetTop;
		}
		//_targetOffsetTop = 0;
	}

	_ready = false;
}


//////////////////////////////////////////////////////////////////////////
void AdScene::scrollToObject(BaseObject *object) {
	if (object) {
		scrollTo(object->_posX, object->_posY - object->getHeight() / 2);
	}
}


//////////////////////////////////////////////////////////////////////////
void AdScene::skipToObject(BaseObject *object) {
	if (object) {
		skipTo(object->_posX, object->_posY - object->getHeight() / 2);
	}
}


//////////////////////////////////////////////////////////////////////////
void AdScene::skipTo(int offsetX, int offsetY) {
	int viewportWidth, viewportHeight;
	getViewportSize(&viewportWidth, &viewportHeight);

	_offsetLeft = MAX(0, offsetX - viewportWidth / 2);
	_offsetLeft = MIN(_offsetLeft, _width - viewportWidth);

	_offsetTop = MAX(0, offsetY - viewportHeight / 2);
	_offsetTop = MIN(_offsetTop, _height - viewportHeight);

	_targetOffsetLeft = _offsetLeft;
	_targetOffsetTop = _offsetTop;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool AdScene::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// LoadActor
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "LoadActor") == 0) {
		stack->correctParams(1);
		AdActor *act = new AdActor(_gameRef);
		if (act && DID_SUCCEED(act->loadFile(stack->pop()->getString()))) {
			addObject(act);
			stack->pushNative(act, true);
		} else {
			delete act;
			act = NULL;
			stack->pushNULL();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadEntity") == 0) {
		stack->correctParams(1);
		AdEntity *ent = new AdEntity(_gameRef);
		if (ent && DID_SUCCEED(ent->loadFile(stack->pop()->getString()))) {
			addObject(ent);
			stack->pushNative(ent, true);
		} else {
			delete ent;
			ent = NULL;
			stack->pushNULL();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateEntity") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		AdEntity *ent = new AdEntity(_gameRef);
		addObject(ent);
		if (!val->isNULL()) {
			ent->setName(val->getString());
		}
		stack->pushNative(ent, true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// UnloadObject / UnloadActor / UnloadEntity / UnloadActor3D / DeleteEntity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "UnloadObject") == 0 || strcmp(name, "UnloadActor") == 0 || strcmp(name, "UnloadEntity") == 0 || strcmp(name, "UnloadActor3D") == 0 || strcmp(name, "DeleteEntity") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		AdObject *obj = (AdObject *)val->getNative();
		removeObject(obj);
		if (val->getType() == VAL_VARIABLE_REF) {
			val->setNULL();
		}

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SkipTo
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SkipTo") == 0) {
		stack->correctParams(2);
		ScValue *val1 = stack->pop();
		ScValue *val2 = stack->pop();
		if (val1->isNative()) {
			skipToObject((BaseObject *)val1->getNative());
		} else {
			skipTo(val1->getInt(), val2->getInt());
		}
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollTo / ScrollToAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScrollTo") == 0 || strcmp(name, "ScrollToAsync") == 0) {
		stack->correctParams(2);
		ScValue *val1 = stack->pop();
		ScValue *val2 = stack->pop();
		if (val1->isNative()) {
			scrollToObject((BaseObject *)val1->getNative());
		} else {
			scrollTo(val1->getInt(), val2->getInt());
		}
		if (strcmp(name, "ScrollTo") == 0) {
			script->waitForExclusive(this);
		}
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetLayer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetLayer") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		if (val->isInt()) {
			int layer = val->getInt();
			if (layer < 0 || layer >= (int32)_layers.size()) {
				stack->pushNULL();
			} else {
				stack->pushNative(_layers[layer], true);
			}
		} else {
			const char *layerName = val->getString();
			bool layerFound = false;
			for (uint32 i = 0; i < _layers.size(); i++) {
				if (scumm_stricmp(layerName, _layers[i]->getName()) == 0) {
					stack->pushNative(_layers[i], true);
					layerFound = true;
					break;
				}
			}
			if (!layerFound) {
				stack->pushNULL();
			}
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetWaypointGroup
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetWaypointGroup") == 0) {
		stack->correctParams(1);
		int group = stack->pop()->getInt();
		if (group < 0 || group >= (int32)_waypointGroups.size()) {
			stack->pushNULL();
		} else {
			stack->pushNative(_waypointGroups[group], true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetNode
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetNode") == 0) {
		stack->correctParams(1);
		const char *nodeName = stack->pop()->getString();

		BaseObject *node = getNodeByName(nodeName);
		if (node) {
			stack->pushNative((BaseScriptable *)node, true);
		} else {
			stack->pushNULL();
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFreeNode
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetFreeNode") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		AdObject *ret = NULL;
		if (val->isInt()) {
			int index = val->getInt();
			if (index >= 0 && index < (int32)_objects.size()) {
				ret = _objects[index];
			}
		} else {
			const char *nodeName = val->getString();
			for (uint32 i = 0; i < _objects.size(); i++) {
				if (_objects[i] && _objects[i]->getName() && scumm_stricmp(_objects[i]->getName(), nodeName) == 0) {
					ret = _objects[i];
					break;
				}
			}
		}
		if (ret) {
			stack->pushNative(ret, true);
		} else {
			stack->pushNULL();
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetRegionAt
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetRegionAt") == 0) {
		stack->correctParams(3);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();
		ScValue *val = stack->pop();

		bool includeDecors = false;
		if (!val->isNULL()) {
			includeDecors = val->getBool();
		}

		if (_mainLayer) {
			for (int i = _mainLayer->_nodes.size() - 1; i >= 0; i--) {
				AdSceneNode *node = _mainLayer->_nodes[i];
				if (node->_type == OBJECT_REGION && node->_region->_active && node->_region->pointInRegion(x, y)) {
					if (node->_region->_decoration && !includeDecors) {
						continue;
					}

					stack->pushNative(node->_region, true);
					return STATUS_OK;
				}
			}
		}
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsBlockedAt
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsBlockedAt") == 0) {
		stack->correctParams(2);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();

		stack->pushBool(isBlockedAt(x, y));
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsWalkableAt
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsWalkableAt") == 0) {
		stack->correctParams(2);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();

		stack->pushBool(isWalkableAt(x, y));
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetScaleAt
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetScaleAt") == 0) {
		stack->correctParams(2);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();

		stack->pushFloat(getZoomAt(x, y));
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetRotationAt
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetRotationAt") == 0) {
		stack->correctParams(2);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();

		stack->pushFloat(getRotationAt(x, y));
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsScrolling
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsScrolling") == 0) {
		stack->correctParams(0);
		bool ret = false;
		if (_autoScroll) {
			if (_targetOffsetLeft != _offsetLeft || _targetOffsetTop != _offsetTop) {
				ret = true;
			}
		}

		stack->pushBool(ret);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeOut / FadeOutAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FadeOut") == 0 || strcmp(name, "FadeOutAsync") == 0) {
		stack->correctParams(5);
		uint32 duration = stack->pop()->getInt(500);
		byte red = stack->pop()->getInt(0);
		byte green = stack->pop()->getInt(0);
		byte blue = stack->pop()->getInt(0);
		byte alpha = stack->pop()->getInt(0xFF);

		_fader->fadeOut(BYTETORGBA(red, green, blue, alpha), duration);
		if (strcmp(name, "FadeOutAsync") != 0) {
			script->waitFor(_fader);
		}

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeIn / FadeInAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FadeIn") == 0 || strcmp(name, "FadeInAsync") == 0) {
		stack->correctParams(5);
		uint32 duration = stack->pop()->getInt(500);
		byte red = stack->pop()->getInt(0);
		byte green = stack->pop()->getInt(0);
		byte blue = stack->pop()->getInt(0);
		byte alpha = stack->pop()->getInt(0xFF);

		_fader->fadeIn(BYTETORGBA(red, green, blue, alpha), duration);
		if (strcmp(name, "FadeInAsync") != 0) {
			script->waitFor(_fader);
		}

		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFadeColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetFadeColor") == 0) {
		stack->correctParams(0);
		stack->pushInt(_fader->getCurrentColor());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsPointInViewport
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsPointInViewport") == 0) {
		stack->correctParams(2);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();
		stack->pushBool(pointInViewport(x, y));
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetViewport
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetViewport") == 0) {
		stack->correctParams(4);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();
		int width = stack->pop()->getInt();
		int height = stack->pop()->getInt();

		if (width <= 0) {
			width = _gameRef->_renderer->_width;
		}
		if (height <= 0) {
			height = _gameRef->_renderer->_height;
		}

		if (!_viewport) {
			_viewport = new BaseViewport(_gameRef);
		}
		if (_viewport) {
			_viewport->setRect(x, y, x + width, y + height);
		}

		stack->pushBool(true);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddLayer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddLayer") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		AdLayer *layer = new AdLayer(_gameRef);
		if (!val->isNULL()) {
			layer->setName(val->getString());
		}
		if (_mainLayer) {
			layer->_width = _mainLayer->_width;
			layer->_height = _mainLayer->_height;
		}
		_layers.add(layer);
		_gameRef->registerObject(layer);

		stack->pushNative(layer, true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InsertLayer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InsertLayer") == 0) {
		stack->correctParams(2);
		int index = stack->pop()->getInt();
		ScValue *val = stack->pop();

		AdLayer *layer = new AdLayer(_gameRef);
		if (!val->isNULL()) {
			layer->setName(val->getString());
		}
		if (_mainLayer) {
			layer->_width = _mainLayer->_width;
			layer->_height = _mainLayer->_height;
		}
		if (index < 0) {
			index = 0;
		}
		if (index <= (int32)_layers.size() - 1) {
			_layers.insert_at(index, layer);
		} else {
			_layers.add(layer);
		}

		_gameRef->registerObject(layer);

		stack->pushNative(layer, true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteLayer
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteLayer") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		AdLayer *toDelete = NULL;
		if (val->isNative()) {
			BaseScriptable *temp = val->getNative();
			for (uint32 i = 0; i < _layers.size(); i++) {
				if (_layers[i] == temp) {
					toDelete = _layers[i];
					break;
				}
			}
		} else {
			int index = val->getInt();
			if (index >= 0 && index < (int32)_layers.size()) {
				toDelete = _layers[index];
			}
		}
		if (toDelete == NULL) {
			stack->pushBool(false);
			return STATUS_OK;
		}

		if (toDelete->_main) {
			script->runtimeError("Scene.DeleteLayer - cannot delete main scene layer");
			stack->pushBool(false);
			return STATUS_OK;
		}

		for (uint32 i = 0; i < _layers.size(); i++) {
			if (_layers[i] == toDelete) {
				_layers.remove_at(i);
				_gameRef->unregisterObject(toDelete);
				break;
			}
		}
		stack->pushBool(true);
		return STATUS_OK;
	} else {
		return BaseObject::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *AdScene::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("scene");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumLayers (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "NumLayers") {
		_scValue->setInt(_layers.size());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumWaypointGroups (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "NumWaypointGroups") {
		_scValue->setInt(_waypointGroups.size());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MainLayer (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "MainLayer") {
		if (_mainLayer) {
			_scValue->setNative(_mainLayer, true);
		} else {
			_scValue->setNULL();
		}

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumFreeNodes (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "NumFreeNodes") {
		_scValue->setInt(_objects.size());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseX (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "MouseX") {
		int viewportX;
		getViewportOffset(&viewportX);

		_scValue->setInt(_gameRef->_mousePos.x + _offsetLeft - viewportX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MouseY (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "MouseY") {
		int viewportY;
		getViewportOffset(NULL, &viewportY);

		_scValue->setInt(_gameRef->_mousePos.y + _offsetTop - viewportY);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoScroll
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AutoScroll") {
		_scValue->setBool(_autoScroll);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PersistentState
	//////////////////////////////////////////////////////////////////////////
	else if (name == "PersistentState") {
		_scValue->setBool(_persistentState);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PersistentStateSprites
	//////////////////////////////////////////////////////////////////////////
	else if (name == "PersistentStateSprites") {
		_scValue->setBool(_persistentStateSprites);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollPixelsX
	//////////////////////////////////////////////////////////////////////////
	else if (name == "ScrollPixelsX") {
		_scValue->setInt(_scrollPixelsH);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollPixelsY
	//////////////////////////////////////////////////////////////////////////
	else if (name == "ScrollPixelsY") {
		_scValue->setInt(_scrollPixelsV);
		return _scValue;
	}


	//////////////////////////////////////////////////////////////////////////
	// ScrollSpeedX
	//////////////////////////////////////////////////////////////////////////
	else if (name == "ScrollSpeedX") {
		_scValue->setInt(_scrollTimeH);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollSpeedY
	//////////////////////////////////////////////////////////////////////////
	else if (name == "ScrollSpeedY") {
		_scValue->setInt(_scrollTimeV);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// OffsetX
	//////////////////////////////////////////////////////////////////////////
	else if (name == "OffsetX") {
		_scValue->setInt(_offsetLeft);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// OffsetY
	//////////////////////////////////////////////////////////////////////////
	else if (name == "OffsetY") {
		_scValue->setInt(_offsetTop);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Width (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Width") {
		if (_mainLayer) {
			_scValue->setInt(_mainLayer->_width);
		} else {
			_scValue->setInt(0);
		}
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Height (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Height") {
		if (_mainLayer) {
			_scValue->setInt(_mainLayer->_height);
		} else {
			_scValue->setInt(0);
		}
		return _scValue;
	} else {
		return BaseObject::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Name") == 0) {
		setName(value->getString());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoScroll
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AutoScroll") == 0) {
		_autoScroll = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PersistentState
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PersistentState") == 0) {
		_persistentState = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PersistentStateSprites
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PersistentStateSprites") == 0) {
		_persistentStateSprites = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollPixelsX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScrollPixelsX") == 0) {
		_scrollPixelsH = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollPixelsY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScrollPixelsY") == 0) {
		_scrollPixelsV = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollSpeedX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScrollSpeedX") == 0) {
		_scrollTimeH = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScrollSpeedY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScrollSpeedY") == 0) {
		_scrollTimeV = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// OffsetX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "OffsetX") == 0) {
		_offsetLeft = value->getInt();

		int viewportWidth, viewportHeight;
		getViewportSize(&viewportWidth, &viewportHeight);

		_offsetLeft = MAX(0, _offsetLeft - viewportWidth / 2);
		_offsetLeft = MIN(_offsetLeft, _width - viewportWidth);
		_targetOffsetLeft = _offsetLeft;

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// OffsetY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "OffsetY") == 0) {
		_offsetTop = value->getInt();

		int viewportWidth, viewportHeight;
		getViewportSize(&viewportWidth, &viewportHeight);

		_offsetTop = MAX(0, _offsetTop - viewportHeight / 2);
		_offsetTop = MIN(_offsetTop, _height - viewportHeight);
		_targetOffsetTop = _offsetTop;

		return STATUS_OK;
	} else {
		return BaseObject::scSetProperty(name, value);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *AdScene::scToString() {
	return "[scene object]";
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::addObject(AdObject *object) {
	_objects.add(object);
	return _gameRef->registerObject(object);
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::removeObject(AdObject *object) {
	for (uint32 i = 0; i < _objects.size(); i++) {
		if (_objects[i] == object) {
			_objects.remove_at(i);
			return _gameRef->unregisterObject(object);
		}
	}
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "SCENE {\n");

	buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", getName());
	buffer->putTextIndent(indent + 2, "CAPTION=\"%s\"\n", getCaption());

	if (_persistentState) {
		buffer->putTextIndent(indent + 2, "PERSISTENT_STATE=%s\n", _persistentState ? "TRUE" : "FALSE");
	}

	if (!_persistentStateSprites) {
		buffer->putTextIndent(indent + 2, "PERSISTENT_STATE_SPRITES=%s\n", _persistentStateSprites ? "TRUE" : "FALSE");
	}


	// scripts
	for (uint32 i = 0; i < _scripts.size(); i++) {
		buffer->putTextIndent(indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	buffer->putTextIndent(indent + 2, "\n");

	// properties
	if (_scProp) {
		_scProp->saveAsText(buffer, indent + 2);
	}

	// viewport
	if (_viewport) {
		Rect32 *rc = _viewport->getRect();
		buffer->putTextIndent(indent + 2, "VIEWPORT { %d, %d, %d, %d }\n", rc->left, rc->top, rc->right, rc->bottom);
	}



	// editor settings
	buffer->putTextIndent(indent + 2, "; ----- editor settings\n");
	buffer->putTextIndent(indent + 2, "EDITOR_MARGIN_H=%d\n", _editorMarginH);
	buffer->putTextIndent(indent + 2, "EDITOR_MARGIN_V=%d\n", _editorMarginV);
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_FRAME { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColFrame), RGBCOLGetG(_editorColFrame), RGBCOLGetB(_editorColFrame), RGBCOLGetA(_editorColFrame));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_ENTITY_SEL { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColEntitySel), RGBCOLGetG(_editorColEntitySel), RGBCOLGetB(_editorColEntitySel), RGBCOLGetA(_editorColEntitySel));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_REGION_SEL { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColRegionSel), RGBCOLGetG(_editorColRegionSel), RGBCOLGetB(_editorColRegionSel), RGBCOLGetA(_editorColRegionSel));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_BLOCKED_SEL { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColBlockedSel), RGBCOLGetG(_editorColBlockedSel), RGBCOLGetB(_editorColBlockedSel), RGBCOLGetA(_editorColBlockedSel));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_DECORATION_SEL { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColDecorSel), RGBCOLGetG(_editorColDecorSel), RGBCOLGetB(_editorColDecorSel), RGBCOLGetA(_editorColDecorSel));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_WAYPOINTS_SEL { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColWaypointsSel), RGBCOLGetG(_editorColWaypointsSel), RGBCOLGetB(_editorColWaypointsSel), RGBCOLGetA(_editorColWaypointsSel));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_ENTITY { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColEntity), RGBCOLGetG(_editorColEntity), RGBCOLGetB(_editorColEntity), RGBCOLGetA(_editorColEntity));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_REGION { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColRegion), RGBCOLGetG(_editorColRegion), RGBCOLGetB(_editorColRegion), RGBCOLGetA(_editorColRegion));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_DECORATION { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColDecor), RGBCOLGetG(_editorColDecor), RGBCOLGetB(_editorColDecor), RGBCOLGetA(_editorColDecor));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_BLOCKED { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColBlocked), RGBCOLGetG(_editorColBlocked), RGBCOLGetB(_editorColBlocked), RGBCOLGetA(_editorColBlocked));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_WAYPOINTS { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColWaypoints), RGBCOLGetG(_editorColWaypoints), RGBCOLGetB(_editorColWaypoints), RGBCOLGetA(_editorColWaypoints));
	buffer->putTextIndent(indent + 2, "EDITOR_COLOR_SCALE { %d,%d,%d,%d }\n", RGBCOLGetR(_editorColScale), RGBCOLGetG(_editorColScale), RGBCOLGetB(_editorColScale), RGBCOLGetA(_editorColScale));

	buffer->putTextIndent(indent + 2, "EDITOR_SHOW_REGIONS=%s\n", _editorShowRegions ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "EDITOR_SHOW_BLOCKED=%s\n", _editorShowBlocked ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "EDITOR_SHOW_DECORATION=%s\n", _editorShowDecor ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "EDITOR_SHOW_ENTITIES=%s\n", _editorShowEntities ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "EDITOR_SHOW_SCALE=%s\n", _editorShowScale ? "TRUE" : "FALSE");

	buffer->putTextIndent(indent + 2, "\n");

	BaseClass::saveAsText(buffer, indent + 2);

	// waypoints
	buffer->putTextIndent(indent + 2, "; ----- waypoints\n");
	for (uint32 i = 0; i < _waypointGroups.size(); i++) {
		_waypointGroups[i]->saveAsText(buffer, indent + 2);
	}

	buffer->putTextIndent(indent + 2, "\n");

	// layers
	buffer->putTextIndent(indent + 2, "; ----- layers\n");
	for (uint32 i = 0; i < _layers.size(); i++) {
		_layers[i]->saveAsText(buffer, indent + 2);
	}

	// scale levels
	buffer->putTextIndent(indent + 2, "; ----- scale levels\n");
	for (uint32 i = 0; i < _scaleLevels.size(); i++) {
		_scaleLevels[i]->saveAsText(buffer, indent + 2);
	}

	// rotation levels
	buffer->putTextIndent(indent + 2, "; ----- rotation levels\n");
	for (uint32 i = 0; i < _rotLevels.size(); i++) {
		_rotLevels[i]->saveAsText(buffer, indent + 2);
	}


	buffer->putTextIndent(indent + 2, "\n");

	// free entities
	buffer->putTextIndent(indent + 2, "; ----- free entities\n");
	for (uint32 i = 0; i < _objects.size(); i++) {
		if (_objects[i]->_type == OBJECT_ENTITY) {
			_objects[i]->saveAsText(buffer, indent + 2);

		}
	}

	buffer->putTextIndent(indent, "}\n");
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::sortScaleLevels() {
	if (_scaleLevels.size() == 0) {
		return STATUS_OK;
	}
	bool changed;
	do {
		changed = false;
		for (uint32 i = 0; i < _scaleLevels.size() - 1; i++) {
			if (_scaleLevels[i]->_posY > _scaleLevels[i + 1]->_posY) {
				AdScaleLevel *sl = _scaleLevels[i];
				_scaleLevels[i] = _scaleLevels[i + 1];
				_scaleLevels[i + 1] = sl;

				changed = true;
			}
		}

	} while (changed);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::sortRotLevels() {
	if (_rotLevels.size() == 0) {
		return STATUS_OK;
	}
	bool changed;
	do {
		changed = false;
		for (uint32 i = 0; i < _rotLevels.size() - 1; i++) {
			if (_rotLevels[i]->_posX > _rotLevels[i + 1]->_posX) {
				AdRotLevel *rl = _rotLevels[i];
				_rotLevels[i] = _rotLevels[i + 1];
				_rotLevels[i + 1] = rl;

				changed = true;
			}
		}

	} while (changed);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
float AdScene::getScaleAt(int Y) {
	AdScaleLevel *prev = NULL;
	AdScaleLevel *next = NULL;

	for (uint32 i = 0; i < _scaleLevels.size(); i++) {
		/* AdScaleLevel *xxx = _scaleLevels[i];*/
		/* int j = _scaleLevels.size(); */
		if (_scaleLevels[i]->_posY < Y) {
			prev = _scaleLevels[i];
		} else {
			next = _scaleLevels[i];
			break;
		}
	}

	if (prev == NULL || next == NULL) {
		return 100;
	}

	int delta_y = next->_posY - prev->_posY;
	float delta_scale = next->_scale - prev->_scale;
	Y -= prev->_posY;

	float percent = (float)Y / ((float)delta_y / 100.0f);
	return prev->_scale + delta_scale / 100 * percent;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::persist(BasePersistenceManager *persistMgr) {
	BaseObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_autoScroll));
	persistMgr->transfer(TMEMBER(_editorColBlocked));
	persistMgr->transfer(TMEMBER(_editorColBlockedSel));
	persistMgr->transfer(TMEMBER(_editorColDecor));
	persistMgr->transfer(TMEMBER(_editorColDecorSel));
	persistMgr->transfer(TMEMBER(_editorColEntity));
	persistMgr->transfer(TMEMBER(_editorColEntitySel));
	persistMgr->transfer(TMEMBER(_editorColFrame));
	persistMgr->transfer(TMEMBER(_editorColRegion));
	persistMgr->transfer(TMEMBER(_editorColRegionSel));
	persistMgr->transfer(TMEMBER(_editorColScale));
	persistMgr->transfer(TMEMBER(_editorColWaypoints));
	persistMgr->transfer(TMEMBER(_editorColWaypointsSel));
	persistMgr->transfer(TMEMBER(_editorMarginH));
	persistMgr->transfer(TMEMBER(_editorMarginV));
	persistMgr->transfer(TMEMBER(_editorShowBlocked));
	persistMgr->transfer(TMEMBER(_editorShowDecor));
	persistMgr->transfer(TMEMBER(_editorShowEntities));
	persistMgr->transfer(TMEMBER(_editorShowRegions));
	persistMgr->transfer(TMEMBER(_editorShowScale));
	persistMgr->transfer(TMEMBER(_fader));
	persistMgr->transfer(TMEMBER(_height));
	persistMgr->transfer(TMEMBER(_initialized));
	persistMgr->transfer(TMEMBER(_lastTimeH));
	persistMgr->transfer(TMEMBER(_lastTimeV));
	_layers.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_mainLayer));
	_objects.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_offsetLeft));
	persistMgr->transfer(TMEMBER(_offsetTop));
	persistMgr->transfer(TMEMBER(_paralaxScrolling));
	persistMgr->transfer(TMEMBER(_persistentState));
	persistMgr->transfer(TMEMBER(_persistentStateSprites));
	persistMgr->transfer(TMEMBER(_pfMaxTime));
	_pfPath.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_pfPointsNum));
	persistMgr->transfer(TMEMBER(_pfReady));
	persistMgr->transfer(TMEMBER(_pfRequester));
	persistMgr->transfer(TMEMBER(_pfTarget));
	persistMgr->transfer(TMEMBER(_pfTargetPath));
	_rotLevels.persist(persistMgr);
	_scaleLevels.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_scrollPixelsH));
	persistMgr->transfer(TMEMBER(_scrollPixelsV));
	persistMgr->transfer(TMEMBER(_scrollTimeH));
	persistMgr->transfer(TMEMBER(_scrollTimeV));
	persistMgr->transfer(TMEMBER(_shieldWindow));
	persistMgr->transfer(TMEMBER(_targetOffsetLeft));
	persistMgr->transfer(TMEMBER(_targetOffsetTop));
	_waypointGroups.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_viewport));
	persistMgr->transfer(TMEMBER(_width));

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdScene::afterLoad() {
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdScene::correctTargetPoint2(int startX, int startY, int *targetX, int *targetY, bool checkFreeObjects, BaseObject *requester) {
	double xStep, yStep, x, y;
	int xLength, yLength, xCount, yCount;
	int x1, y1, x2, y2;

	x1 = *targetX;
	y1 = *targetY;
	x2 = startX;
	y2 = startY;


	xLength = abs(x2 - x1);
	yLength = abs(y2 - y1);

	if (xLength > yLength) {

		yStep = fabs((double)(y2 - y1) / (double)(x2 - x1));
		y = y1;

		for (xCount = x1; xCount < x2; xCount++) {
			if (isWalkableAt(xCount, (int)y, checkFreeObjects, requester)) {
				*targetX = xCount;
				*targetY = (int)y;
				return STATUS_OK;
			}
			y += yStep;
		}
	} else {

		xStep = fabs((double)(x2 - x1) / (double)(y2 - y1));
		x = x1;

		for (yCount = y1; yCount < y2; yCount++) {
			if (isWalkableAt((int)x, yCount, checkFreeObjects, requester)) {
				*targetX = (int)x;
				*targetY = yCount;
				return STATUS_OK;
			}
			x += xStep;
		}
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdScene::correctTargetPoint(int startX, int startY, int *argX, int *argY, bool checkFreeObjects, BaseObject *requester) {
	int x = *argX;
	int y = *argY;

	if (isWalkableAt(x, y, checkFreeObjects, requester) || !_mainLayer) {
		return STATUS_OK;
	}

	// right
	int lengthRight = 0;
	bool foundRight = false;
	for (x = *argX, y = *argY; x < _mainLayer->_width; x++, lengthRight++) {
		if (isWalkableAt(x, y, checkFreeObjects, requester) && isWalkableAt(x - 5, y, checkFreeObjects, requester)) {
			foundRight = true;
			break;
		}
	}

	// left
	int lengthLeft = 0;
	bool foundLeft = false;
	for (x = *argX, y = *argY; x >= 0; x--, lengthLeft--) {
		if (isWalkableAt(x, y, checkFreeObjects, requester) && isWalkableAt(x + 5, y, checkFreeObjects, requester)) {
			foundLeft = true;
			break;
		}
	}

	// up
	int lengthUp = 0;
	bool foundUp = false;
	for (x = *argX, y = *argY; y >= 0; y--, lengthUp--) {
		if (isWalkableAt(x, y, checkFreeObjects, requester) && isWalkableAt(x, y + 5, checkFreeObjects, requester)) {
			foundUp = true;
			break;
		}
	}

	// down
	int lengthDown = 0;
	bool foundDown = false;
	for (x = *argX, y = *argY; y < _mainLayer->_height; y++, lengthDown++) {
		if (isWalkableAt(x, y, checkFreeObjects, requester) && isWalkableAt(x, y - 5, checkFreeObjects, requester)) {
			foundDown = true;
			break;
		}
	}

	if (!foundLeft && !foundRight && !foundUp && !foundDown) {
		return STATUS_OK;
	}

	int offsetX = INT_MAX, offsetY = INT_MAX;

	if (foundLeft && foundRight) {
		if (abs(lengthLeft) < abs(lengthRight)) {
			offsetX = lengthLeft;
		} else {
			offsetX = lengthRight;
		}
	} else if (foundLeft) {
		offsetX = lengthLeft;
	} else if (foundRight) {
		offsetX = lengthRight;
	}

	if (foundUp && foundDown) {
		if (abs(lengthUp) < abs(lengthDown)) {
			offsetY = lengthUp;
		} else {
			offsetY = lengthDown;
		}
	} else if (foundUp) {
		offsetY = lengthUp;
	} else if (foundDown) {
		offsetY = lengthDown;
	}

	if (abs(offsetX) < abs(offsetY)) {
		*argX = *argX + offsetX;
	} else {
		*argY = *argY + offsetY;
	}

	if (!isWalkableAt(*argX, *argY)) {
		return correctTargetPoint2(startX, startY, argX, argY, checkFreeObjects, requester);
	} else {
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
void AdScene::pfPointsStart() {
	_pfPointsNum = 0;
}


//////////////////////////////////////////////////////////////////////////
void AdScene::pfPointsAdd(int x, int y, int distance) {
	if (_pfPointsNum >= (int32)_pfPath.size()) {
		_pfPath.add(new AdPathPoint(x, y, distance));
	} else {
		_pfPath[_pfPointsNum]->x = x;
		_pfPath[_pfPointsNum]->y = y;
		_pfPath[_pfPointsNum]->_distance = distance;
		_pfPath[_pfPointsNum]->_marked = false;
		_pfPath[_pfPointsNum]->_origin = NULL;
	}

	_pfPointsNum++;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::getViewportOffset(int *offsetX, int *offsetY) {
	AdGame *adGame = (AdGame *)_gameRef;
	if (_viewport && !_gameRef->_editorMode) {
		if (offsetX) {
			*offsetX = _viewport->_offsetX;
		}
		if (offsetY) {
			*offsetY = _viewport->_offsetY;
		}
	} else if (adGame->_sceneViewport && !_gameRef->_editorMode) {
		if (offsetX) {
			*offsetX = adGame->_sceneViewport->_offsetX;
		}
		if (offsetY) {
			*offsetY = adGame->_sceneViewport->_offsetY;
		}
	} else {
		if (offsetX) {
			*offsetX = 0;
		}
		if (offsetY) {
			*offsetY = 0;
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::getViewportSize(int *width, int *height) {
	AdGame *adGame = (AdGame *)_gameRef;
	if (_viewport && !_gameRef->_editorMode) {
		if (width) {
			*width  = _viewport->getWidth();
		}
		if (height) {
			*height = _viewport->getHeight();
		}
	} else if (adGame->_sceneViewport && !_gameRef->_editorMode) {
		if (width) {
			*width  = adGame->_sceneViewport->getWidth();
		}
		if (height) {
			*height = adGame->_sceneViewport->getHeight();
		}
	} else {
		if (width) {
			*width  = _gameRef->_renderer->_width;
		}
		if (height) {
			*height = _gameRef->_renderer->_height;
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
int AdScene::getOffsetLeft() {
	int viewportX;
	getViewportOffset(&viewportX);

	return _offsetLeft - viewportX;
}


//////////////////////////////////////////////////////////////////////////
int AdScene::getOffsetTop() {
	int viewportY;
	getViewportOffset(NULL, &viewportY);

	return _offsetTop - viewportY;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::pointInViewport(int x, int y) {
	int left, top, width, height;

	getViewportOffset(&left, &top);
	getViewportSize(&width, &height);

	return x >= left && x <= left + width && y >= top && y <= top + height;
}


//////////////////////////////////////////////////////////////////////////
void AdScene::setOffset(int offsetLeft, int offsetTop) {
	_offsetLeft = offsetLeft;
	_offsetTop  = offsetTop;
}


//////////////////////////////////////////////////////////////////////////
BaseObject *AdScene::getNodeByName(const char *name) {
	BaseObject *ret = NULL;

	// dependent objects
	for (uint32 i = 0; i < _layers.size(); i++) {
		AdLayer *layer = _layers[i];
		for (uint32 j = 0; j < layer->_nodes.size(); j++) {
			AdSceneNode *node = layer->_nodes[j];
			if ((node->_type == OBJECT_ENTITY && !scumm_stricmp(name, node->_entity->getName())) ||
			        (node->_type == OBJECT_REGION && !scumm_stricmp(name, node->_region->getName()))) {
				switch (node->_type) {
				case OBJECT_ENTITY:
					ret = node->_entity;
					break;
				case OBJECT_REGION:
					ret = node->_region;
					break;
				default:
					ret = NULL;
				}
				return ret;
			}
		}
	}

	// free entities
	for (uint32 i = 0; i < _objects.size(); i++) {
		if (_objects[i]->_type == OBJECT_ENTITY && !scumm_stricmp(name, _objects[i]->getName())) {
			return _objects[i];
		}
	}

	// waypoint groups
	for (uint32 i = 0; i < _waypointGroups.size(); i++) {
		if (!scumm_stricmp(name, _waypointGroups[i]->getName())) {
			return _waypointGroups[i];
		}
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::saveState() {
	return persistState(true);
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::loadState() {
	return persistState(false);
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::persistState(bool saving) {
	if (!_persistentState) {
		return STATUS_OK;
	}

	AdGame *adGame = (AdGame *)_gameRef;
	AdSceneState *state = adGame->getSceneState(getFilename(), saving);
	if (!state) {
		return STATUS_OK;
	}

	AdNodeState *nodeState;

	// dependent objects
	for (uint32 i = 0; i < _layers.size(); i++) {
		AdLayer *layer = _layers[i];
		for (uint32 j = 0; j < layer->_nodes.size(); j++) {
			AdSceneNode *node = layer->_nodes[j];
			switch (node->_type) {
			case OBJECT_ENTITY:
				if (!node->_entity->_saveState) {
					continue;
				}
				nodeState = state->getNodeState(node->_entity->getName(), saving);
				if (nodeState) {
					nodeState->transferEntity(node->_entity, _persistentStateSprites, saving);
					//if (Saving) NodeState->_active = node->_entity->_active;
					//else node->_entity->_active = NodeState->_active;
				}
				break;
			case OBJECT_REGION:
				if (!node->_region->_saveState) {
					continue;
				}
				nodeState = state->getNodeState(node->_region->getName(), saving);
				if (nodeState) {
					if (saving) {
						nodeState->_active = node->_region->_active;
					} else {
						node->_region->_active = nodeState->_active;
					}
				}
				break;
			default:
				warning("AdScene::PersistState - unhandled enum");
				break;
			}
		}
	}

	// free entities
	for (uint32 i = 0; i < _objects.size(); i++) {
		if (!_objects[i]->_saveState) {
			continue;
		}
		if (_objects[i]->_type == OBJECT_ENTITY) {
			nodeState = state->getNodeState(_objects[i]->getName(), saving);
			if (nodeState) {
				nodeState->transferEntity((AdEntity *)_objects[i], _persistentStateSprites, saving);
				//if (Saving) NodeState->_active = _objects[i]->_active;
				//else _objects[i]->_active = NodeState->_active;
			}
		}
	}

	// waypoint groups
	for (uint32 i = 0; i < _waypointGroups.size(); i++) {
		nodeState = state->getNodeState(_waypointGroups[i]->getName(), saving);
		if (nodeState) {
			if (saving) {
				nodeState->_active = _waypointGroups[i]->_active;
			} else {
				_waypointGroups[i]->_active = nodeState->_active;
			}
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
float AdScene::getRotationAt(int x, int y) {
	AdRotLevel *prev = NULL;
	AdRotLevel *next = NULL;

	for (uint32 i = 0; i < _rotLevels.size(); i++) {
		/*  AdRotLevel *xxx = _rotLevels[i];
		    int j = _rotLevels.size();*/
		if (_rotLevels[i]->_posX < x) {
			prev = _rotLevels[i];
		} else {
			next = _rotLevels[i];
			break;
		}
	}

	if (prev == NULL || next == NULL) {
		return 0;
	}

	int delta_x = next->_posX - prev->_posX;
	float delta_rot = next->_rotation - prev->_rotation;
	x -= prev->_posX;

	float percent = (float)x / ((float)delta_x / 100.0f);
	return prev->_rotation + delta_rot / 100 * percent;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::handleItemAssociations(const char *itemName, bool show) {
	for (uint32 i = 0; i < _layers.size(); i++) {
		AdLayer *layer = _layers[i];
		for (uint32 j = 0; j < layer->_nodes.size(); j++) {
			if (layer->_nodes[j]->_type == OBJECT_ENTITY) {
				AdEntity *ent = layer->_nodes[j]->_entity;

				if (ent->_item && strcmp(ent->_item, itemName) == 0) {
					ent->_active = show;
				}
			}
		}
	}

	for (uint32 i = 0; i < _objects.size(); i++) {
		if (_objects[i]->_type == OBJECT_ENTITY) {
			AdEntity *ent = (AdEntity *)_objects[i];
			if (ent->_item && strcmp(ent->_item, itemName) == 0) {
				ent->_active = show;
			}
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::getRegionsAt(int x, int y, AdRegion **regionList, int numRegions) {
	int numUsed = 0;
	if (_mainLayer) {
		for (int i = _mainLayer->_nodes.size() - 1; i >= 0; i--) {
			AdSceneNode *node = _mainLayer->_nodes[i];
			if (node->_type == OBJECT_REGION && node->_region->_active && node->_region->pointInRegion(x, y)) {
				if (numUsed < numRegions - 1) {
					regionList[numUsed] = node->_region;
					numUsed++;
				} else {
					break;
				}
			}
		}
	}
	for (int i = numUsed; i < numRegions; i++) {
		regionList[i] = NULL;
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdScene::restoreDeviceObjects() {
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
BaseObject *AdScene::getNextAccessObject(BaseObject *currObject) {
	BaseArray<AdObject *> objects;
	getSceneObjects(objects, true);

	if (objects.size() == 0) {
		return NULL;
	} else {
		if (currObject != NULL) {
			for (uint32 i = 0; i < objects.size(); i++) {
				if (objects[i] == currObject) {
					if (i < objects.size() - 1) {
						return objects[i + 1];
					} else {
						break;
					}
				}
			}
		}
		return objects[0];
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
BaseObject *AdScene::getPrevAccessObject(BaseObject *currObject) {
	BaseArray<AdObject *> objects;
	getSceneObjects(objects, true);

	if (objects.size() == 0) {
		return NULL;
	} else {
		if (currObject != NULL) {
			for (int i = objects.size() - 1; i >= 0; i--) {
				if (objects[i] == currObject) {
					if (i > 0) {
						return objects[i - 1];
					} else {
						break;
					}
				}
			}
		}
		return objects[objects.size() - 1];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::getSceneObjects(BaseArray<AdObject *> &objects, bool interactiveOnly) {
	for (uint32 i = 0; i < _layers.size(); i++) {
		// close-up layer -> remove everything below it
		if (interactiveOnly && _layers[i]->_closeUp) {
			objects.clear();
		}


		for (uint32 j = 0; j < _layers[i]->_nodes.size(); j++) {
			AdSceneNode *node = _layers[i]->_nodes[j];
			switch (node->_type) {
			case OBJECT_ENTITY: {
				AdEntity *ent = node->_entity;
				if (ent->_active && (ent->_registrable || !interactiveOnly)) {
					objects.add(ent);
				}
			}
			break;

			case OBJECT_REGION: {
				BaseArray<AdObject *> regionObj;
				getRegionObjects(node->_region, regionObj, interactiveOnly);
				for (uint32 newIndex = 0; newIndex < regionObj.size(); newIndex++) {
					bool found = false;
					for (uint32 old = 0; old < objects.size(); old++) {
						if (objects[old] == regionObj[newIndex]) {
							found = true;
							break;
						}
					}
					if (!found) {
						objects.add(regionObj[newIndex]);
					}
				}
				//if (regionObj.size() > 0) Objects.Append(RegionObj);
			}
			break;
			default:
				debugC(kWintermuteDebugGeneral, "AdScene::GetSceneObjects - Unhandled enum");
				break;
			}
		}
	}

	// objects outside any region
	BaseArray<AdObject *> regionObj;
	getRegionObjects(NULL, regionObj, interactiveOnly);
	for (uint32 newIndex = 0; newIndex < regionObj.size(); newIndex++) {
		bool found = false;
		for (uint32 old = 0; old < objects.size(); old++) {
			if (objects[old] == regionObj[newIndex]) {
				found = true;
				break;
			}
		}
		if (!found) {
			objects.add(regionObj[newIndex]);
		}
	}


	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdScene::getRegionObjects(AdRegion *region, BaseArray<AdObject *> &objects, bool interactiveOnly) {
	AdGame *adGame = (AdGame *)_gameRef;
	AdObject *obj;

	// global objects
	for (uint32 i = 0; i < adGame->_objects.size(); i++) {
		obj = adGame->_objects[i];
		if (obj->_active && (obj->_stickRegion == region || region == NULL || (obj->_stickRegion == NULL && region->pointInRegion(obj->_posX, obj->_posY)))) {
			if (interactiveOnly && !obj->_registrable) {
				continue;
			}

			objects.add(obj);
		}
	}

	// scene objects
	for (uint32 i = 0; i < _objects.size(); i++) {
		obj = _objects[i];
		if (obj->_active && !obj->_editorOnly && (obj->_stickRegion == region || region == NULL || (obj->_stickRegion == NULL && region->pointInRegion(obj->_posX, obj->_posY)))) {
			if (interactiveOnly && !obj->_registrable) {
				continue;
			}

			objects.add(obj);
		}
	}

	// sort by _posY
	Common::sort(objects.begin(), objects.end(), AdScene::compareObjs);

	return STATUS_OK;
}

} // end of namespace Wintermute
