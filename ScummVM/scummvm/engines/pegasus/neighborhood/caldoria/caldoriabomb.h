/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#ifndef PEGASUS_NEIGHBORHOOD_CALDORIA_CALDORIABOMB_H
#define PEGASUS_NEIGHBORHOOD_CALDORIA_CALDORIABOMB_H

#include "pegasus/interaction.h"
#include "pegasus/notification.h"
#include "pegasus/surface.h"

namespace Pegasus {

/*
	Edge list is arranged as follows:

	all values in the edge list are bytes.

	all vertices are numbers between 0 and 24. x coordinate of vertex is vertex % 5,
	and y coordinate is vertex / 5.

	an edge is
		a direction code
		a number of vertices in the edge
		an array of vertices -- all vertices along the edge, whether or not they're
			clickable.
		an array of bools (bytes) indicating that a portion of the edge is
			traversed (and should be drawn). the number of bools is one less than
			the number of vertices.

	an edge list is
		an array of 25 bools indicating which vertex is clickable.
		an array of 25 bools indicating which vertex is used (drawn).
		a number of edges
		an array of edges.

	a hot vertex list is
		a number of vertices
		an array of 25 vertices

*/

typedef int8 VertexType;
typedef VertexType *BombEdgeList;

static const VertexType kEdgeOneSixteenth = 0;
static const VertexType kEdgeOneEighth = 1;
static const VertexType kEdgeThreeSixteenths = 2;
static const VertexType kEdgeOneFourth = 3;
static const VertexType kEdgeFiveSixteenths = 4;
static const VertexType kEdgeThreeEighths = 5;
static const VertexType kEdgeSevenSixteenths = 6;
static const VertexType kEdgeOneHalf = 7;

class BombTimer : public IdlerAnimation {
public:
	BombTimer(const DisplayElementID);
	virtual ~BombTimer() {}

	void draw(const Common::Rect &);

protected:
	void timeChanged(const TimeValue);

	int _middle;
	Surface _leftImage, _rightImage;
};

class BombGrid : public Picture {
public:
	BombGrid(const DisplayElementID);
	virtual ~BombGrid() {}

	void drawEdges(BombEdgeList);

protected:
	Frame _yellowDot;
	Frame _yellowOneSixteenth;
	Frame _yellowOneEighth;
	Frame _yellowThreeSixteenths;
	Frame _yellowOneFourth;
	Frame _yellowFiveSixteenths;
	Frame _yellowThreeEighths;
	Frame _yellowSevenSixteenths;
	Frame _yellowOneHalf;
	Frame _redDot;
	Frame _redOneSixteenth;
	Frame _redOneEighth;
	Frame _redThreeSixteenths;
	Frame _redOneFourth;
	Frame _redFiveSixteenths;
	Frame _redThreeEighths;
	Frame _redSevenSixteenths;
	Frame _redOneHalf;
};

class Hotspot;

class CaldoriaBomb : public GameInteraction, public NotificationReceiver {
public:
	CaldoriaBomb(Neighborhood *, NotificationManager *);
	virtual ~CaldoriaBomb();

	long getNumHints();
	Common::String getHintMovie(uint);
	void doSolve();
	bool canSolve();

protected:
	void openInteraction();
	void initInteraction();
	void closeInteraction();
	void receiveNotification(Notification *, const NotificationFlags);
	void activateHotspots();
	void clickInHotspot(const Input &, const Hotspot *);
	void handleInput(const Input &, const Hotspot *);
	InputBits getInputFilter();
	void startBombAmbient(Common::String);

	Notification *_neighborhoodNotification;
	BombGrid _grid;
	BombTimer _timer;
	BombEdgeList _bombLevel[6];
	int _currentLevel, _flashTime;
	Hotspot *_vertexHotspot[25];
	VertexType _lastVertex;
	Notification _timerNotification;
	NotificationCallBack _timerCallBack;

	TimeValue _readTime;
};

} // End of namespace Pegasus

#endif
