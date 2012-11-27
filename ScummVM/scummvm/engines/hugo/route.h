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
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef HUGO_ROUTE_H
#define HUGO_ROUTE_H

#include "common/rect.h"

namespace Hugo {

/**
 * Purpose of an automatic route
 */
enum RouteType {kRouteSpace, kRouteExit, kRouteLook, kRouteGet};

struct Segment {                                    // Search segment
	int16 _y;                                       // y position
	int16 _x1, _x2;                                 // Range of segment
};

class Route {
public:
	Route(HugoEngine *vm);

	void  resetRoute();
	int16 getRouteIndex() const;

	void processRoute();
	bool startRoute(const RouteType routeType, const int16 objId, int16 cx, int16 cy);
	void setDirection(const uint16 keyCode);
	void setWalk(const uint16 direction);

private:
	HugoEngine *_vm;

	static const int kMapBound = 1;                 // Mark a boundary outline
	static const int kMapFill = 2;                  // Mark a boundary filled
	static const int kMaxSeg = 256;                 // Maximum number of segments
	static const int kMaxNodes = 256;               // Maximum nodes in route

	uint16 _oldWalkDirection;                       // Last direction char

	int16     _routeIndex;                          // Index into route list, or -1
	RouteType _routeType;                           // Purpose of an automatic route
	int16     _routeObjId;                          // Index of exit of object walking to

	byte _boundaryMap[kYPix][kXPix];                // Boundary byte map
	Segment _segment[kMaxSeg];                      // List of points in fill-path
	Common::Point _route[kMaxNodes];                // List of nodes in route (global)
	int16 _segmentNumb;                             // Count number of segments
	int16 _routeListIndex;                          // Index into route list
	int16 _destX;
	int16 _destY;
	int16 _heroWidth;                               // Hero width
	bool  _routeFoundFl;                            // TRUE when path found
	bool  _fullStackFl;                             // TRUE if stack exhausted
	bool  _fullSegmentFl;                           // Segments exhausted

	void segment(int16 x, int16 y);
	bool findRoute(const int16 cx, const int16 cy);
	Common::Point *newNode();
};

} // End of namespace Hugo

#endif //HUGO_ROUTE_H
