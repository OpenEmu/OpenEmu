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

#ifndef WINTERMUTE_BASE_ACTIVE_RECT_H
#define WINTERMUTE_BASE_ACTIVE_RECT_H

#include "engines/wintermute/math/rect32.h"
#include "engines/wintermute/base/base.h"

namespace Wintermute {
class BaseRegion;
class BaseSubFrame;
class BaseObject;
class BaseActiveRect: BaseClass {
public:
	void clipRect();
	bool _precise;
	float _zoomX;
	float _zoomY;
	BaseSubFrame *_frame;
	BaseObject *_owner;
	BaseRegion *_region;
	int _offsetX;
	int _offsetY;
	Rect32 _rect;
	BaseActiveRect(BaseGame *inGameOwner = NULL);
	BaseActiveRect(BaseGame *inGameOwner, BaseObject *owner, BaseSubFrame *frame, int x, int y, int width, int height, float zoomX = 100, float zoomY = 100, bool precise = true);
	BaseActiveRect(BaseGame *inGame, BaseObject *owner, BaseRegion *region, int offsetX, int offsetY);
	virtual ~BaseActiveRect();

};

} // end of namespace Wintermute

#endif
