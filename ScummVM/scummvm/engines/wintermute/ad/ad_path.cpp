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

#include "engines/wintermute/ad/ad_path.h"
#include "engines/wintermute/base/base_point.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdPath, false)

//////////////////////////////////////////////////////////////////////////
AdPath::AdPath(BaseGame *inGame) : BaseClass(inGame) {
	_currIndex = -1;
	_ready = false;
}


//////////////////////////////////////////////////////////////////////////
AdPath::~AdPath() {
	reset();
}


//////////////////////////////////////////////////////////////////////////
void AdPath::reset() {
	for (uint32 i = 0; i < _points.size(); i++) {
		delete _points[i];
	}

	_points.clear();
	_currIndex = -1;
	_ready = false;
}


//////////////////////////////////////////////////////////////////////////
BasePoint *AdPath::getFirst() {
	if (_points.size() > 0) {
		_currIndex = 0;
		return _points[_currIndex];
	} else {
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
BasePoint *AdPath::getNext() {
	_currIndex++;
	if (_currIndex < (int32)_points.size()) {
		return _points[_currIndex];
	} else {
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
BasePoint *AdPath::getCurrent() {
	if (_currIndex >= 0 && _currIndex < (int32)_points.size()) {
		return _points[_currIndex];
	} else {
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
void AdPath::addPoint(BasePoint *point) {
	_points.add(point);
}


//////////////////////////////////////////////////////////////////////////
bool AdPath::setReady(bool ready) {
	bool orig = _ready;
	_ready = ready;

	return orig;
}


//////////////////////////////////////////////////////////////////////////
bool AdPath::persist(BasePersistenceManager *persistMgr) {

	persistMgr->transfer(TMEMBER(_gameRef));

	persistMgr->transfer(TMEMBER(_currIndex));
	_points.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_ready));

	return STATUS_OK;
}

} // end of namespace Wintermute
