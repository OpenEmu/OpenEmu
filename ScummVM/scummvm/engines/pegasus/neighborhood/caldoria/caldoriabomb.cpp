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

#include "pegasus/gamestate.h"
#include "pegasus/pegasus.h"
#include "pegasus/neighborhood/caldoria/caldoria.h"
#include "pegasus/neighborhood/caldoria/caldoriabomb.h"

namespace Pegasus {

// Bomb game PICTs:

static const uint16 kYellowBombPICTBaseID = 700;
static const uint16 kRedBombPICTBaseID = 709;
static const uint16 kTimerLeftPICTID = 718;
static const uint16 kTimerRightPICTID = 719;

static const uint32 kFlashOnTime = 20;
static const uint32 kFlashOffTime = 10;

static const uint32 kOnTime1 = kFlashOnTime;
static const uint32 kOffTime1 = kOnTime1 + kFlashOffTime;
static const uint32 kOnTime2 = kOffTime1 + kFlashOnTime;
static const uint32 kOffTime2 = kOnTime2 + kFlashOffTime;
static const uint32 kOnTime3 = kOffTime2 + kFlashOnTime;
static const uint32 kOffTime3 = kOnTime3 + kFlashOffTime;
static const uint32 kOnTime4 = kOffTime3 + kFlashOnTime;

static const HotSpotID kVertextHotSpotBaseID = 10000;

static const CoordType kVertextHotSpotWidth = 24;
static const CoordType kVertextHotSpotHeight = 24;

static const NotificationFlags kBombTimerExpiredFlag = 1;

static const VertexType kBombLevelOne[] = {
	0, 1, 0, 1, 0,			// hot vertices first.
	1, 1, 0, 1, 1,
	1, 1, 0, 1, 0,
	1, 1, 0, 1, 1,
	0, 1, 0, 1, 0,

	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,

	9,						// 9 edges in this level

	kEdgeOneFourth,
	3,
	1, 2, 3,
	0, 0,

	kEdgeOneFourth,
	5,
	5, 6, 7, 8, 9,
	0, 0, 0, 0,

	kEdgeOneFourth,
	4,
	10, 11, 12, 13,
	0, 0, 0,

	kEdgeOneFourth,
	5,
	15, 16, 17, 18, 19,
	0, 0, 0, 0,

	kEdgeOneFourth,
	3,
	21, 22, 23,
	0, 0,

	kEdgeOneHalf,
	3,
	5, 10, 15,
	0, 0,

	kEdgeOneHalf,
	5,
	1, 6, 11, 16, 21,
	0, 0, 0, 0,

	kEdgeOneHalf,
	5,
	3, 8, 13, 18, 23,
	0, 0, 0, 0,

	kEdgeOneHalf,
	3,
	9, 14, 19,
	0, 0
};

static const VertexType kBombLevelTwo[] = {
	0, 1, 0, 1, 0,
	1, 1, 1, 0, 1,
	0, 0, 0, 1, 0,
	1, 1, 1, 0, 1,
	0, 1, 0, 1, 0,

	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,

	15,

	kEdgeOneEighth,
	2,
	5, 1,
	0,

	kEdgeOneEighth,
	3,
	17, 13, 9,
	0, 0,

	kEdgeOneEighth,
	2,
	23, 19,
	0,

	kEdgeThreeEighths,
	2,
	3, 9,
	0,

	kEdgeThreeEighths,
	3,
	7, 13, 19,
	0, 0,

	kEdgeThreeEighths,
	2,
	15, 21,
	0,

	kEdgeOneFourth,
	3,
	1, 2, 3,
	0, 0,

	kEdgeOneFourth,
	4,
	6, 7, 8, 9,
	0, 0, 0,

	kEdgeOneFourth,
	4,
	16, 17, 18, 19,
	0, 0, 0,

	kEdgeOneFourth,
	3,
	21, 22, 23,
	0, 0,

	kEdgeOneHalf,
	3,
	5, 10, 15,
	0, 0,

	kEdgeOneHalf,
	2,
	1, 6,
	0,

	kEdgeOneHalf,
	3,
	7, 12, 17,
	0, 0,

	kEdgeOneHalf,
	3,
	9, 14, 19,
	0, 0,

	kEdgeOneHalf,
	2,
	16, 21,
	0
};

static const VertexType kBombLevelThree[] = {
	0, 1, 0, 1, 0,
	1, 1, 1, 1, 1,
	0, 1, 1, 0, 0,
	1, 1, 1, 1, 1,
	0, 1, 0, 1, 0,

	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,

	22,

	kEdgeThreeSixteenths,
	3,
	15, 12, 9,
	0, 0,

	kEdgeFiveSixteenths,
	3,
	5, 12, 19,
	0, 0,

	kEdgeOneEighth,
	2,
	5, 1,
	0,

	kEdgeOneEighth,
	2,
	7, 3,
	0,

	kEdgeOneEighth,
	2,
	15, 11,
	0,

	kEdgeOneEighth,
	2,
	21, 17,
	0,

	kEdgeOneEighth,
	2,
	23, 19,
	0,

	kEdgeThreeEighths,
	2,
	1, 7,
	0,

	kEdgeThreeEighths,
	2,
	3, 9,
	0,

	kEdgeThreeEighths,
	2,
	5, 11,
	0,

	kEdgeThreeEighths,
	2,
	15, 21,
	0,

	kEdgeThreeEighths,
	2,
	17, 23,
	0,

	kEdgeOneFourth,
	3,
	1, 2, 3,
	0, 0,

	kEdgeOneFourth,
	2,
	5, 6,
	0,

	kEdgeOneFourth,
	2,
	8, 9,
	0,

	kEdgeOneFourth,
	2,
	15, 16,
	0,

	kEdgeOneFourth,
	2,
	18, 19,
	0,

	kEdgeOneFourth,
	3,
	21, 22, 23,
	0, 0,

	kEdgeOneHalf,
	2,
	1, 6,
	0,

	kEdgeOneHalf,
	2,
	3, 8,
	0,

	kEdgeOneHalf,
	2,
	16, 21,
	0,

	kEdgeOneHalf,
	2,
	18, 23,
	0
};

static const VertexType kBombLevelFour[] = {
	1, 1, 1, 1, 0,
	1, 1, 0, 1, 1,
	1, 0, 1, 0, 1,
	1, 1, 0, 1, 1,
	0, 1, 1, 1, 1,

	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,

	19,

	kEdgeOneEighth,
	2,
	5, 1,
	0,

	kEdgeOneEighth,
	3,
	10, 6, 2,
	0, 0,

	kEdgeOneEighth,
	3,
	16, 12, 8,
	0, 0,

	kEdgeOneEighth,
	3,
	22, 18, 14,
	0, 0,

	kEdgeOneEighth,
	2,
	23, 19,
	0,

	kEdgeThreeEighths,
	3,
	2, 8, 14,
	0, 0,

	kEdgeThreeEighths,
	3,
	10, 16, 22,
	0, 0,

	kEdgeOneFourth,
	4,
	0, 1, 2, 3,
	0, 0, 0,

	kEdgeOneFourth,
	2,
	5, 6,
	0,

	kEdgeOneFourth,
	2,
	8, 9,
	0,

	kEdgeOneFourth,
	2,
	15, 16,
	0,

	kEdgeOneFourth,
	2,
	18, 19,
	0,

	kEdgeOneFourth,
	4,
	21, 22, 23, 24,
	0, 0, 0,

	kEdgeOneHalf,
	4,
	0, 5, 10, 15,
	0, 0, 0,

	kEdgeOneHalf,
	2,
	1, 6,
	0,

	kEdgeOneHalf,
	2,
	3, 8,
	0,

	kEdgeOneHalf,
	4,
	9, 14, 19, 24,
	0, 0, 0,

	kEdgeOneHalf,
	2,
	16, 21,
	0,

	kEdgeOneHalf,
	2,
	18, 23,
	0
};

static const VertexType kBombLevelFive[] = {
	0, 1, 0, 1, 0,
	1, 1, 1, 1, 1,
	0, 1, 1, 1, 0,
	1, 1, 1, 1, 1,
	0, 1, 0, 1, 0,

	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,

	19,

	kEdgeOneEighth,
	2,
	5, 1,
	0,

	kEdgeOneEighth,
	2,
	7, 3,
	0,

	kEdgeOneEighth,
	2,
	13, 9,
	0,

	kEdgeOneEighth,
	2,
	15, 11,
	0,

	kEdgeOneEighth,
	2,
	21, 17,
	0,

	kEdgeOneEighth,
	2,
	23, 19,
	0,

	kEdgeThreeEighths,
	2,
	1, 7,
	0,

	kEdgeThreeEighths,
	4,
	5, 11, 17, 23,
	0, 0, 0,

	kEdgeThreeEighths,
	3,
	6, 12, 18,
	0, 0,

	kEdgeThreeEighths,
	2,
	13, 19,
	0,

	kEdgeThreeEighths,
	2,
	15, 21,
	0,

	kEdgeOneFourth,
	5,
	5, 6, 7, 8, 9,
	0, 0, 0, 0,

	kEdgeOneFourth,
	3,
	15, 16, 17,
	0, 0,

	kEdgeOneFourth,
	2,
	18, 19,
	0,

	kEdgeOneFourth,
	3,
	21, 22, 23,
	0, 0,

	kEdgeOneHalf,
	3,
	5, 10, 15,
	0, 0,

	kEdgeOneHalf,
	2,
	1, 6,
	0,

	kEdgeOneHalf,
	3,
	11, 16, 21,
	0, 0,

	kEdgeOneHalf,
	5,
	3, 8, 13, 18, 23,
	0, 0, 0, 0
};

static const VertexType kBombLevelSix[] = {
	0, 1, 1, 1, 0,
	1, 1, 1, 1, 1,
	1, 0, 0, 0, 1,
	1, 1, 1, 1, 1,
	0, 1, 1, 1, 0,

	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,

	25,

	kEdgeOneSixteenth,
	2,
	10, 1,
	0,

	kEdgeOneSixteenth,
	2,
	23, 14,
	0,

	kEdgeSevenSixteenths,
	2,
	3, 14,
	0,

	kEdgeSevenSixteenths,
	2,
	10, 21,
	0,

	kEdgeOneEighth,
	2,
	5, 1,
	0,

	kEdgeOneEighth,
	3,
	10, 6, 2,
	0, 0,

	kEdgeOneEighth,
	2,
	7, 3,
	0,

	kEdgeOneEighth,
	2,
	21, 17,
	0,

	kEdgeOneEighth,
	3,
	22, 18, 14,
	0, 0,

	kEdgeOneEighth,
	2,
	23, 19,
	0,

	kEdgeThreeEighths,
	2,
	1, 7,
	0,

	kEdgeThreeEighths,
	3,
	2, 8, 14,
	0, 0,

	kEdgeThreeEighths,
	2,
	3, 9,
	0,

	kEdgeThreeEighths,
	3,
	10, 16, 22,
	0, 0,

	kEdgeThreeEighths,
	2,
	15, 21,
	0,

	kEdgeThreeEighths,
	2,
	17, 23,
	0,

	kEdgeOneFourth,
	3,
	1, 2, 3,
	0, 0,

	kEdgeOneFourth,
	3,
	6, 7, 8,
	0, 0,

	kEdgeOneFourth,
	3,
	16, 17, 18,
	0, 0,

	kEdgeOneFourth,
	3,
	21, 22, 23,
	0, 0,

	kEdgeOneHalf,
	3,
	5, 10, 15,
	0, 0,

	kEdgeOneHalf,
	3,
	6, 11, 16,
	0, 0,

	kEdgeOneHalf,
	5,
	2, 7, 12, 17, 22,
	0, 0, 0, 0,

	kEdgeOneHalf,
	3,
	8, 13, 18,
	0, 0,

	kEdgeOneHalf,
	3,
	9, 14, 19,
	0, 0
};

static const CoordType kBombGridWidth = 140;
static const CoordType kBombGridHeight = 140;

static const CoordType kDotOriginX = 0;
static const CoordType kDotOriginY = 0;

static const CoordType kVertOriginX = 2;
static const CoordType kVertOriginY = 6;

static const CoordType kHorizOriginX = 6;
static const CoordType kHorizOriginY = 2;

static const CoordType kDiagOriginX = 6;
static const CoordType kDiagOriginY = 6;

static const int g_originsX[] = {
	kDiagOriginX,
	kDiagOriginX,
	kDiagOriginX,
	kHorizOriginX,
	kDiagOriginX,
	kDiagOriginX,
	kDiagOriginX,
	kVertOriginX
};

static const int g_originsY[] = {
	kDiagOriginY - 64,
	kDiagOriginY - 32,
	kDiagOriginY - 32,
	kHorizOriginY,
	kDiagOriginY,
	kDiagOriginY,
	kDiagOriginY,
	kVertOriginY
};

struct HotVerticesList {
	int numHotVerts;
	VertexType hotVerts[25];
};

CoordType vertToX(VertexType vertex) {
	return (vertex % 5) * 32;
}

CoordType vertToY(VertexType vertex) {
	return (vertex / 5) * 32;
}

// This function returns the number of edges in the bomb edge list.
VertexType getNumEdges(BombEdgeList edges) {
	return edges[50];
}

// These four functions return pointers into the given edge list.

// getFirstEdge and getNextEdge can be used to iterate across all edges
// in an edge list. These functions can be used to walk all the edges
// in a bomb edge list for drawing.
VertexType *getFirstEdge(BombEdgeList edges) {
	return &edges[51];
}

VertexType *getNextEdge(VertexType *anEdge) {
	return anEdge + *(anEdge + 1) * 2 + 1;
}

// getVertices returns a pointer to all of the vertices that should are
// hot. These vertices indicate all the vertices that should be drawn in
// the game.
VertexType *getVertices(BombEdgeList edges) {
	return &edges[0];
}

// getUsedVertices returns a pointer to the "used" vertices area: the
// area that keeps track of which vertices have been set by the
// setVertexUsed used function.
VertexType *getUsedVertices(BombEdgeList edges) {
	return &edges[25];
}

// Useful for saving. Saving the state of the bomb game is as simple as writing
// out the edge list.
int getEdgeListSize(BombEdgeList edges) {
	VertexType numEdges = getNumEdges(edges);
	VertexType *anEdge = getFirstEdge(edges);

	while (numEdges--)
		anEdge = getNextEdge(anEdge);

	return anEdge - edges + 4;
}

// Returns true if the given vertex lies on the given edge.
bool vertexOnEdge(VertexType *anEdge, VertexType whichVertex) {
	VertexType numVerts = *++anEdge;

	while (numVerts--)
		if (*++anEdge == whichVertex)
			return true;

	return false;
}

// Given an edge list and a from vertex, this function constructs a list
// of all vertices that may be clicked on.
// if fromVertex == -1, all vertices are eligible.
// otherwise, only vertices on a line from fromVertex are eligible.
void makeHotVertexList(BombEdgeList edges, VertexType fromVertex, HotVerticesList &hotVertices) {
	hotVertices.numHotVerts = 0;

	if (fromVertex == -1) {
		for (VertexType i = 0; i < 25; i++)
			if (edges[i])
				hotVertices.hotVerts[hotVertices.numHotVerts++] = i;
	} else {
		VertexType numEdges = getNumEdges(edges);
		VertexType *anEdge = getFirstEdge(edges);
		hotVertices.hotVerts[hotVertices.numHotVerts++] = fromVertex;

		while (numEdges--) {
			if (vertexOnEdge(anEdge, fromVertex)) {
				VertexType *p = anEdge + 1;
				VertexType numVerts = *p;

				while (numVerts--)
					if (*++p != fromVertex)
						hotVertices.hotVerts[hotVertices.numHotVerts++] = *p;
			}

			anEdge = getNextEdge(anEdge);
		}
	}
}

// Set all edges in the edge list to the value passed in "edgeVal".
// For drawing purposes, 0 can mean don't draw, and 1 and higher can
// represent different colors.
void setAllEdgesUsed(BombEdgeList edges, VertexType used) {
	VertexType numEdges = getNumEdges(edges);
	VertexType *anEdge = getFirstEdge(edges);

	while (numEdges--) {
		VertexType *p1 = anEdge + 1;
		VertexType numVerts = *p1;
		p1 += numVerts + 1;

		while (--numVerts)
			*p1++ = used;

		anEdge = getNextEdge(anEdge);
	}

	VertexType *p1 = edges;
	VertexType *p2 = getUsedVertices(edges);

	for (VertexType i = 0; i < 25; i++, p1++, p2++)
		if (*p1)
			*p2 = used;
}

// Same as setAllEdgesUsed, but only affects edges that are already set
// to a non-zero value.
void setAllUsedEdgesUsed(BombEdgeList edges, VertexType used) {
	VertexType numEdges = getNumEdges(edges);
	VertexType *anEdge = getFirstEdge(edges);

	while (numEdges--) {
		VertexType *p = anEdge + 1;
		VertexType numVerts = *p;
		p += numVerts + 1;

		while (--numVerts) {
			if (*p)
				*p = used;
			++p;
		}

		anEdge = getNextEdge(anEdge);
	}

	VertexType *p = getUsedVertices(edges);
	for (VertexType i = 0; i < 25; i++, p++)
		if (*p)
			*p = used;
}

// Replace all edges with value "value" with the new value "used".
void replaceUsedEdges(BombEdgeList edges, VertexType value, VertexType used) {
	VertexType numEdges = getNumEdges(edges);
	VertexType *anEdge = getFirstEdge(edges);

	while (numEdges--) {
		VertexType *p = anEdge + 1;
		VertexType numVerts = *p;
		p += numVerts + 1;

		while (--numVerts) {
			if (*p == value)
				*p = used;

			p++;
		}

		anEdge = getNextEdge(anEdge);
	}

	VertexType *p = getUsedVertices(edges);
	for (VertexType i = 0; i < 25; i++, p++)
		if (*p == value)
			*p = used;
}

// Set a vertex's value to "used".
void setVertexUsed(BombEdgeList edges, VertexType whichVertex, VertexType value) {
	*(getUsedVertices(edges) + whichVertex) = value;
}

// Mark an edge in the given list between the two vertices as "used". This marks
// all inbetween vertices as well, even if the vertex is not marked as a "hot"
// vertex in the hot vertex section. Returns true if doing this operation
// crosses an already marked edge.
bool setEdgeUsed(BombEdgeList edges, VertexType fromVertex, VertexType toVertex) {
	VertexType numEdges = getNumEdges(edges);
	VertexType *anEdge = getFirstEdge(edges);
	bool crossed = false;

	while (numEdges--) {
		VertexType *p = anEdge;
		VertexType numVerts = *++p;
		VertexType *fromPtr = 0;
		VertexType *toPtr = 0;
		VertexType i = numVerts;
		p++;

		while (i--) {
			if (*p == fromVertex)
				fromPtr = p;
			else if (*p == toVertex)
				toPtr = p;

			if (fromPtr && toPtr) {
				// Found the edge...
				if (fromPtr > toPtr) {
					p = fromPtr;
					fromPtr = toPtr;
					toPtr = p;
				}

				p = fromPtr + numVerts;

				for (i = toPtr - fromPtr; i > 0; i--, p++) {
					++(*p);

					if (*p == 2)
						crossed = true;
				}

				VertexType *verts = getVertices(edges);
				VertexType *usedVerts = getUsedVertices(edges);
				*(usedVerts + *fromPtr) = 1;

				for (p = fromPtr + 1; p != toPtr; p++)
					if (*(verts + *p))
						*(usedVerts + *p) = 1;

				*(usedVerts + *toPtr) = 1;
				return crossed;
			}

			p++;
		}

		anEdge = getNextEdge(anEdge);
	}

	return false;
}

// Return true if all edges are used. Can be used to determine when the bomb
// game is over.
bool allEdgesUsed(BombEdgeList edges) {
	VertexType numEdges = getNumEdges(edges);
	VertexType *anEdge = getFirstEdge(edges);

	while (numEdges--) {
		VertexType *p = anEdge + 1;
		VertexType numVerts = *p;
		p += numVerts + 1;

		while (--numVerts) {
			if (!*p)
				return false;

			++p;
		}

		anEdge = getNextEdge(anEdge);
	}

	return true;
}

BombGrid::BombGrid(const DisplayElementID id) : Picture(id) {
	Common::Rect bounds(0, 0, kBombGridWidth, kBombGridHeight);

	allocateSurface(bounds);
	setBounds(bounds);
	_surface->fillRect(bounds, g_system->getScreenFormat().RGBToColor(0xff, 0xff, 0xff));

	_transparent = true;

	_yellowDot.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kYellowBombPICTBaseID, true);
	_yellowOneSixteenth.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kYellowBombPICTBaseID + 1, true);
	_yellowOneEighth.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kYellowBombPICTBaseID + 2, true);
	_yellowThreeSixteenths.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kYellowBombPICTBaseID + 3, true);
	_yellowOneFourth.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kYellowBombPICTBaseID + 4, true);
	_yellowFiveSixteenths.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kYellowBombPICTBaseID + 5, true);
	_yellowThreeEighths.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kYellowBombPICTBaseID + 6, true);
	_yellowSevenSixteenths.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kYellowBombPICTBaseID + 7, true);
	_yellowOneHalf.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kYellowBombPICTBaseID + 8, true);

	_redDot.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kRedBombPICTBaseID, true);
	_redOneSixteenth.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kRedBombPICTBaseID + 1, true);
	_redOneEighth.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kRedBombPICTBaseID + 2, true);
	_redThreeSixteenths.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kRedBombPICTBaseID + 3, true);
	_redOneFourth.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kRedBombPICTBaseID + 4, true);
	_redFiveSixteenths.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kRedBombPICTBaseID + 5, true);
	_redThreeEighths.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kRedBombPICTBaseID + 6, true);
	_redSevenSixteenths.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kRedBombPICTBaseID + 7, true);
	_redOneHalf.initFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kRedBombPICTBaseID + 8, true);
}

void BombGrid::drawEdges(BombEdgeList edges) {
	GraphicsManager *gfx = ((PegasusEngine *)g_engine)->_gfx;
	gfx->setCurSurface(_surface);

	_surface->fillRect(Common::Rect(0, 0, kBombGridWidth, kBombGridHeight), g_system->getScreenFormat().RGBToColor(0xff, 0xff, 0xff));

	Frame *yellowStuff = &_yellowDot;
	Frame *redStuff = &_redDot;
	VertexType numEdges = getNumEdges(edges);
	VertexType *anEdge = getFirstEdge(edges);
	VertexType i, *p;

	Common::Rect bounds;
	getSurfaceBounds(bounds);

	while (numEdges--) {
		p = anEdge;
		VertexType edgeDirection = *p++;
		VertexType numVerts = *p++;
		VertexType numSegs = numVerts - 1;

		for (i = 0; i < numSegs; i++, p++) {
			if (*(p + numVerts) > 0 && *(p + numVerts) < 4) {
				Frame *drawStuff;

				if (*(p + numVerts) == 2)
					drawStuff = redStuff;
				else
					drawStuff = yellowStuff;

				int x = vertToX(*p) + g_originsX[edgeDirection];
				int y = vertToY(*p) + g_originsY[edgeDirection];

				Common::Rect r1;
				drawStuff[edgeDirection + 1].getSurfaceBounds(r1);
				Common::Rect r2 = r1;
				r2.moveTo(x, y);
				drawStuff[edgeDirection + 1].drawImage(r1, r2);
			}
		}

		anEdge = getNextEdge(anEdge);
	}

	for (i = 0, p = getUsedVertices(edges); i < 25; i++, p++) {
		if (*p > 0 && *p < 4) {
			Frame *drawStuff;

			if (*p == 2)
				drawStuff = redStuff;
			else
				drawStuff = yellowStuff;

			int x = vertToX(i) + kDotOriginX;
			int y = vertToY(i) + kDotOriginY;

			Common::Rect r1;
			drawStuff->getSurfaceBounds(r1);
			Common::Rect r2 = r1;
			r2.moveTo(x, y);
			drawStuff->drawImage(r1, r2);
		}
	}

	triggerRedraw();
	gfx->setCurSurface(gfx->getWorkArea());
}

BombTimer::BombTimer(const DisplayElementID id) : IdlerAnimation(id) {
	_middle = -1;
	_leftImage.getImageFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kTimerLeftPICTID);
	_rightImage.getImageFromPICTResource(((PegasusEngine *)g_engine)->_resFork, kTimerRightPICTID);

	Common::Rect r;
	_leftImage.getSurfaceBounds(r);
	setBounds(r);
}

void BombTimer::draw(const Common::Rect &updateRect) {
	Common::Rect bounds;
	getBounds(bounds);

	Common::Rect r1 = bounds;
	r1.right = _middle;
	r1 = r1.findIntersectingRect(updateRect);

	if (!r1.isEmpty()) {
		Common::Rect r2 = r1;
		r2.moveTo(r1.left - bounds.left, r1.top - bounds.top);
		_leftImage.copyToCurrentPort(r2, r1);
	}

	r1 = bounds;
	r1.left = _middle;
	r1 = r1.findIntersectingRect(updateRect);

	if (!r1.isEmpty()) {
		Common::Rect r2 = r1;
		r2.moveTo(r1.left - bounds.left, r1.top - bounds.top);
		_rightImage.copyToCurrentPort(r2, r1);
	}
}

void BombTimer::timeChanged(const TimeValue newTime) {
	Common::Rect bounds;
	getBounds(bounds);

	int newMiddle = bounds.right - bounds.width() * newTime / getDuration();
	if (newMiddle != _middle) {
		_middle = newMiddle;
		triggerRedraw();
	}
}

#define CREATE_BOMB_LEVEL(num, data) \
	_bombLevel[num] = new VertexType[sizeof(data)]; \
	memcpy(_bombLevel[num], data, sizeof(data))

CaldoriaBomb::CaldoriaBomb(Neighborhood *owner, NotificationManager *manager) :
		GameInteraction(kCaldoriaBombInteractionID, owner), _grid(kCaldoriaBombGridID),
		_timer(kCaldoriaBombTimerID), _timerNotification(kCaldoriaBombTimerNotificationID, manager) {
	CREATE_BOMB_LEVEL(0, kBombLevelOne);
	CREATE_BOMB_LEVEL(1, kBombLevelTwo);
	CREATE_BOMB_LEVEL(2, kBombLevelThree);
	CREATE_BOMB_LEVEL(3, kBombLevelFour);
	CREATE_BOMB_LEVEL(4, kBombLevelFive);
	CREATE_BOMB_LEVEL(5, kBombLevelSix);
	_currentLevel = 0;
}

#undef CREATE_BOMB_LEVEL

CaldoriaBomb::~CaldoriaBomb() {
	for (int i = 0; i < 6; i++)
		delete[] _bombLevel[i];
}

void CaldoriaBomb::openInteraction() {
	_grid.moveElementTo(kCaldoriaBombGridLeft, kCaldoriaBombGridTop);
	_grid.setDisplayOrder(kCaldoriaBombGridOrder);
	_grid.startDisplaying();

	_timer.moveElementTo(kCaldoriaBombTimerLeft, kCaldoriaBombTimerTop);
	_timer.setDisplayOrder(kCaldoriaBombTimerOrder);
	_timer.startDisplaying();
	_timer.setSegment(0, kTenMinutesPerFifteenTicks, kFifteenTicksPerSecond);
	_timer.setTime(0);

	_timerNotification.notifyMe(this, kBombTimerExpiredFlag, kBombTimerExpiredFlag);
	_timerCallBack.setNotification(&_timerNotification);
	_timerCallBack.initCallBack(&_timer, kCallBackAtExtremes);
	_timerCallBack.setCallBackFlag(kBombTimerExpiredFlag);

	Common::Rect r(0, 0, kVertextHotSpotWidth, kVertextHotSpotHeight);

	for (VertexType i = 0; i < 25; i++) {
		_vertexHotspot[i] = new Hotspot(i + kVertextHotSpotBaseID);
		r.moveTo(vertToX(i) + kCaldoriaBombGridLeft - kVertextHotSpotWidth / 2 + 6,
				vertToY(i) + kCaldoriaBombGridTop - kVertextHotSpotHeight / 2 + 6);
		_vertexHotspot[i]->setArea(r);
		_vertexHotspot[i]->setHotspotFlags(kNeighborhoodSpotFlag | kClickSpotFlag);
		g_allHotspots.push_back(_vertexHotspot[i]);
	}

	_neighborhoodNotification = _owner->getNeighborhoodNotification();
	_neighborhoodNotification->notifyMe(this, kExtraCompletedFlag, kExtraCompletedFlag);
}

void CaldoriaBomb::initInteraction() {
	_owner->loadLoopSound1("");
	_owner->startExtraSequence(kCaldoria56BombStage1, kExtraCompletedFlag, kFilterNoInput);
}

void CaldoriaBomb::closeInteraction() {
	_timer.stop();
	_timer.hide();
	_timer.stopDisplaying();
	_grid.hide();
	_grid.stopDisplaying();

	// The original did not do this, but we need it here
	// Not sure why the original worked without this; probably
	// related to the way the List code worked in CodeWarrior.
	// If this is not here, the notifications will later attempt
	// to remove itself from this receiver causing a very nasty
	// crash.
	_timerNotification.cancelNotification(this);
	_neighborhoodNotification->cancelNotification(this);
}

void CaldoriaBomb::startBombAmbient(Common::String ambient) {
	_owner->loadLoopSound1(ambient);
}

void CaldoriaBomb::receiveNotification(Notification *notification, const NotificationFlags) {
	if (notification == _neighborhoodNotification) {
		switch (_owner->getLastExtra()) {
		case kCaldoria56BombStage1:
			_grid.show();
			_timer.show();
			_timerCallBack.scheduleCallBack(kTriggerAtStop, 0, 0);
			_timer.start();
			_currentLevel = 0;
			_lastVertex = -1;
			startBombAmbient("Sounds/Caldoria/BmbLoop1.22K.AIFF");
			break;
		case kCaldoria56BombStage2:
		case kCaldoria56BombStage3:
		case kCaldoria56BombStage4:
		case kCaldoria56BombStage5:
		case kCaldoria56BombStage6:
			_grid.show();
			_currentLevel++;
			_grid.drawEdges(_bombLevel[_currentLevel]);
			_lastVertex = -1;
			startBombAmbient(Common::String::format("Sounds/Caldoria/BmbLoop%d.22K.AIFF", _owner->getLastExtra() - kCaldoria56BombStage1 + 1));
			break;
		case kCaldoria56BombStage7:
			_owner->requestDeleteCurrentInteraction();
			GameState.setCaldoriaBombDisarmed(true);
			GameState.setScoringDisarmedNuke(true);
			_owner->loadAmbientLoops();
			break;
		}
	} else if (notification == &_timerNotification) {
		_grid.hide();
		_timer.stop();
		_timer.hide();
		_owner->loadLoopSound1("");
		_owner->playDeathExtra(kCaldoria56BombExplodes, kDeathNuclearExplosion);
	}
}

void CaldoriaBomb::activateHotspots() {
	GameInteraction::activateHotspots();

	if (_currentLevel != -1 && _lastVertex >= -1) {
		HotVerticesList hotVertices;
		makeHotVertexList(_bombLevel[_currentLevel], _lastVertex, hotVertices);

		for (VertexType i = 0; i < hotVertices.numHotVerts; i++)
			g_allHotspots.activateOneHotspot(hotVertices.hotVerts[i] + kVertextHotSpotBaseID);
	}
}

void CaldoriaBomb::clickInHotspot(const Input &input, const Hotspot *hotspot) {
	int clickedVertex = (int)hotspot->getObjectID() - (int)kVertextHotSpotBaseID;

	if (clickedVertex >= 0 && clickedVertex < 25) {
		if (_lastVertex != -1 && setEdgeUsed(_bombLevel[_currentLevel], _lastVertex, clickedVertex)) {
			clickedVertex = -2;
			_flashTime = tickCount();
		} else if (allEdgesUsed(_bombLevel[_currentLevel])) {
			setVertexUsed(_bombLevel[_currentLevel], clickedVertex, 1);
			clickedVertex = -20;
			_flashTime = tickCount();
		} else {
			setVertexUsed(_bombLevel[_currentLevel], clickedVertex, 2);
		}

		_grid.drawEdges(_bombLevel[_currentLevel]);
		_lastVertex = clickedVertex;
	} else {
		GameInteraction::clickInHotspot(input, hotspot);
	}
}

InputBits CaldoriaBomb::getInputFilter() {
	// Disallow arrow buttons.
	return GameInteraction::getInputFilter() & kFilterAllButtons;
}

void CaldoriaBomb::handleInput(const Input &input, const Hotspot *hotspot) {
	GameInteraction::handleInput(input, hotspot);

	switch (_lastVertex) {
	case -2:			// Flash back to yellow.
		if (tickCount() > _flashTime + kOnTime1) {
			replaceUsedEdges(_bombLevel[_currentLevel], 2, 3);
			_grid.drawEdges(_bombLevel[_currentLevel]);
			_lastVertex = -3;
		}
		break;
	case -3:			// Flash back to red.
		if (tickCount() > _flashTime + kOffTime1) {
			replaceUsedEdges(_bombLevel[_currentLevel], 3, 2);
			_grid.drawEdges(_bombLevel[_currentLevel]);
			_lastVertex = -4;
		}
		break;
	case -4:			// Flash all to yellow.
		if (tickCount() > _flashTime + kOnTime2) {
			setAllUsedEdgesUsed(_bombLevel[_currentLevel], 1);
			_grid.drawEdges(_bombLevel[_currentLevel]);
			_lastVertex = -5;
		}
		break;
	case -5:			// Flash all to red.
		if (tickCount() > _flashTime + kOffTime2) {
			setAllUsedEdgesUsed(_bombLevel[_currentLevel], 2);
			_grid.drawEdges(_bombLevel[_currentLevel]);
			_lastVertex = -6;
		}
		break;
	case -6:			// Flash all to yellow.
		if (tickCount() > _flashTime + kOnTime3) {
			setAllUsedEdgesUsed(_bombLevel[_currentLevel], 1);
			_grid.drawEdges(_bombLevel[_currentLevel]);
			_lastVertex = -7;
		}
		break;
	case -7:			// Flash all to red.
		if (tickCount() > _flashTime + kOffTime3) {
			setAllUsedEdgesUsed(_bombLevel[_currentLevel], 2);
			_grid.drawEdges(_bombLevel[_currentLevel]);
			_lastVertex = -8;
		}
		break;
	case -8:			// Restore to normal.
		if (tickCount() > _flashTime + kOnTime4) {
			setAllEdgesUsed(_bombLevel[_currentLevel], 0);
			_grid.drawEdges(_bombLevel[_currentLevel]);
			_lastVertex = -1;
		}
		break;

	// Flash grid after success.
	case -20:			// Flash off.
		if (tickCount() > _flashTime + kOnTime1) {
			setAllEdgesUsed(_bombLevel[_currentLevel], 4);
			_grid.drawEdges(_bombLevel[_currentLevel]);
			_lastVertex = -21;
		}
		break;
	case -21:			// Flash on.
		if (tickCount() > _flashTime + kOffTime1) {
			setAllEdgesUsed(_bombLevel[_currentLevel], 1);
			_grid.drawEdges(_bombLevel[_currentLevel]);
			_lastVertex = -22;
		}
		break;
	case -22:			// Flash off.
		if (tickCount() > _flashTime + kOnTime2) {
			setAllEdgesUsed(_bombLevel[_currentLevel], 4);
			_grid.drawEdges(_bombLevel[_currentLevel]);
			_lastVertex = -23;
		}
		break;
	case -23:			// Flash on.
		if (tickCount() > _flashTime + kOffTime2) {
			setAllEdgesUsed(_bombLevel[_currentLevel], 1);
			_grid.drawEdges(_bombLevel[_currentLevel]);
			_lastVertex = -24;
		}
		break;
	case -24:
		if (tickCount() > _flashTime + kOnTime3) {
			_grid.hide();
			_lastVertex = -1;
			_owner->loadLoopSound1("");

			switch (_currentLevel) {
			case 0:
				_owner->startExtraSequence(kCaldoria56BombStage2, kExtraCompletedFlag, kFilterNoInput);
				break;
			case 1:
				_owner->startExtraSequence(kCaldoria56BombStage3, kExtraCompletedFlag, kFilterNoInput);
				break;
			case 2:
				_owner->startExtraSequence(kCaldoria56BombStage4, kExtraCompletedFlag, kFilterNoInput);
				break;
			case 3:
				_owner->startExtraSequence(kCaldoria56BombStage5, kExtraCompletedFlag, kFilterNoInput);
				break;
			case 4:
				_owner->startExtraSequence(kCaldoria56BombStage6, kExtraCompletedFlag, kFilterNoInput);
				break;
			case 5:
				_timer.stop();
				_grid.hide();
				_timer.hide();
				_owner->startExtraSequence(kCaldoria56BombStage7, kExtraCompletedFlag, kFilterNoInput);
				break;
			}
		}
		break;
	}
}

long CaldoriaBomb::getNumHints() {
	return 2;
}

Common::String CaldoriaBomb::getHintMovie(uint hintNum) {
	return (hintNum == 1) ? "Images/AI/Caldoria/X56EH2" : "Images/AI/Caldoria/X56EH3";
}

bool CaldoriaBomb::canSolve() {
	return true;
}

void CaldoriaBomb::doSolve() {
	_timer.stop();
	_grid.hide();
	_timer.hide();
	_owner->loadLoopSound1("");
	_owner->startExtraSequence(kCaldoria56BombStage7, kExtraCompletedFlag, kFilterNoInput);
}

} // End of namespace Pegasus
