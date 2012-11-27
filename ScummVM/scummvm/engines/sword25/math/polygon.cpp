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
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include <math.h>

#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/inputpersistenceblock.h"

#include "sword25/math/polygon.h"
#include "sword25/math/line.h"

namespace Sword25 {

Polygon::Polygon() : vertexCount(0), vertices(NULL) {
}

Polygon::Polygon(int vertexCount_, const Vertex *vertices_) : vertexCount(0), vertices(NULL) {
	init(vertexCount_, vertices_);
}

Polygon::Polygon(const Polygon &other) : Persistable(other), vertexCount(0), vertices(NULL) {
	init(other.vertexCount, other.vertices);
}

Polygon::Polygon(InputPersistenceBlock &Reader) : vertexCount(0), vertices(NULL) {
	unpersist(Reader);
}

Polygon::~Polygon() {
	delete[] vertices;
}

bool Polygon::init(int vertexCount_, const Vertex *vertices_) {
	// Rember the old obstate to restore it if an error occurs whilst initializing it with the new data
	int oldvertexCount = this->vertexCount;
	Vertex *oldvertices = this->vertices;

	this->vertexCount = vertexCount_;
	this->vertices = new Vertex[vertexCount_ + 1];
	memcpy(this->vertices, vertices_, sizeof(Vertex) * vertexCount_);
	// TODO:
	// Duplicate and remove redundant vertecies (Superflous = 3 co-linear verts)
	// _WeedRepeatedvertices();
	// The first vertex is repeated at the end of the vertex array; this simplifies
	// some algorithms, running through the edges and thus can save the overflow control.
	this->vertices[vertexCount_] = this->vertices[0];

	// If the polygon is self-intersecting, the object state is restore, and an error signalled
	if (checkForSelfIntersection()) {
		delete[] this->vertices;
		this->vertices = oldvertices;
		this->vertexCount = oldvertexCount;

		// BS_LOG_ERROR("POLYGON: Tried to create a self-intersecting polygon.\n");
		return false;
	}

	// Release old vertex list
	delete[] oldvertices;

	// Calculate properties of the polygon
	_isCW = computeIsCW();
	_centroid = computeCentroid();

	return true;
}

// Review the order of the vertices
// ---------------------------------

bool Polygon::isCW() const {
	return _isCW;
}
bool Polygon::computeIsCW() const {
	if (vertexCount) {
		// Find the vertex on extreme bottom right
		int v2Index = findLRVertexIndex();

		// Find the vertex before and after it
		int v1Index = (v2Index + (vertexCount - 1)) % vertexCount;
		int v3Index = (v2Index + 1) % vertexCount;

		// Cross product form
		// If the cross product of the vertex lying fartherest bottom left is positive,
		// the vertecies arrranged in a clockwise order. Otherwise counter-clockwise
		if (crossProduct(vertices[v1Index], vertices[v2Index], vertices[v3Index]) >= 0)
			return true;
	}

	return false;
}

int Polygon::findLRVertexIndex() const {
	if (vertexCount) {
		int curIndex = 0;
		int maxX = vertices[0].x;
		int maxY = vertices[0].y;

		for (int i = 1; i < vertexCount; i++) {
			if (vertices[i].y > maxY ||
			        (vertices[i].y == maxY && vertices[i].x > maxX)) {
				maxX = vertices[i].x;
				maxY = vertices[i].y;
				curIndex = i;
			}
		}

		return curIndex;
	}

	return -1;
}
// Make a determine vertex order
// -----------------------------

void Polygon::ensureCWOrder() {
	if (!isCW())
		reverseVertexOrder();
}
// Reverse the order of vertecies
// ------------------------------

void Polygon::reverseVertexOrder() {
	// vertices are exchanged in pairs, until the list has been completely reversed
	for (int i = 0; i < vertexCount / 2; i++) {
		Vertex tempVertex = vertices[i];
		vertices[i] = vertices[vertexCount - i - 1];
		vertices[vertexCount - i - 1] = tempVertex;
	}

	// Vertexordnung neu berechnen.
	_isCW = computeIsCW();
}

// Cross Product
// -------------

int Polygon::crossProduct(const Vertex &v1, const Vertex &v2, const Vertex &v3) const {
	return (v2.x - v1.x) * (v3.y - v2.y) -
	       (v2.y - v1.y) * (v3.x - v2.x);
}
// Check for self-intersections
// ----------------------------

bool Polygon::checkForSelfIntersection() const {
	// TODO: Finish this
	/*
	float AngleSum = 0.0f;
	for (int i = 0; i < vertexCount; i++) {
	    int j = (i + 1) % vertexCount;
	    int k = (i + 2) % vertexCount;

	    float Dot = DotProduct(vertices[i], vertices[j], vertices[k]);

	    // Skalarproduct normalisieren
	    float Length1 = sqrt((vertices[i].x - vertices[j].x) * (vertices[i].x - vertices[j].x) +
	                         (vertices[i].y - vertices[j].y) * (vertices[i].y - vertices[j].y));
	    float Length2 = sqrt((vertices[k].x - vertices[j].x) * (vertices[k].x - vertices[j].x) +
	                         (vertices[k].y - vertices[j].y) * (vertices[k].y - vertices[j].y));
	    float Norm = Length1 * Length2;

	    if (Norm > 0.0f) {
	        Dot /= Norm;
	        AngleSum += acos(Dot);
	    }
	}
	*/

	return false;
}

// Move
// ----

void Polygon::operator+=(const Vertex &delta) {
	// Move all vertecies
	for (int i = 0; i < vertexCount; i++)
		vertices[i] += delta;

	// Shift the focus
	_centroid += delta;
}

// Line of Sight
// -------------

bool Polygon::isLineInterior(const Vertex &a, const Vertex &b) const {
	// Both points have to be in the polygon
	if (!isPointInPolygon(a, true) || !isPointInPolygon(b, true))
		return false;

	// If the points are identical, the line is trivially within the polygon
	if (a == b)
		return true;

	// Test whether the line intersects a line segment strictly (proper intersection)
	for (int i = 0; i < vertexCount; i++) {
		int j = (i + 1) % vertexCount;
		const Vertex &vs = vertices[i];
		const Vertex &ve = vertices[j];

		// If the line intersects a line segment strictly (proper cross section) the line is not in the polygon
		if (Line::doesIntersectProperly(a, b, vs, ve))
			return false;

		// If one of the two line items is on the edge and the other is to the right of the edge,
		// then the line is not completely within the polygon
		if (Line::isOnLineStrict(vs, ve, a) && Line::isVertexRight(vs, ve, b))
			return false;
		if (Line::isOnLineStrict(vs, ve, b) && Line::isVertexRight(vs, ve, a))
			return false;

		// If one of the two line items is on a vertex, the line traces into the polygon
		if ((a == vs) && !isLineInCone(i, b, true))
			return false;
		if ((b == vs) && !isLineInCone(i, a, true))
			return false;
	}

	return true;
}

bool Polygon::isLineExterior(const Vertex &a, const Vertex &b) const {
	// Neither of the two points must be strictly in the polygon (on the edge is allowed)
	if (isPointInPolygon(a, false) || isPointInPolygon(b, false))
		return false;

	// If the points are identical, the line is trivially outside of the polygon
	if (a == b)
		return true;

	// Test whether the line intersects a line segment strictly (proper intersection)
	for (int i = 0; i < vertexCount; i++) {
		int j = (i + 1) % vertexCount;
		const Vertex &vs = vertices[i];
		const Vertex &ve = vertices[j];

		// If the line intersects a line segment strictly (proper intersection), then
		// the line is partially inside the polygon
		if (Line::doesIntersectProperly(a, b, vs, ve))
			return false;

		// If one of the two line items is on the edge and the other is to the right of the edge,
		// the line is not completely outside the polygon
		if (Line::isOnLineStrict(vs, ve, a) && Line::isVertexLeft(vs, ve, b))
			return false;
		if (Line::isOnLineStrict(vs, ve, b) && Line::isVertexLeft(vs, ve, a))
			return false;

		// If one of the lwo line items is on a vertex, the line must not run into the polygon
		if ((a == vs) && isLineInCone(i, b, false))
			return false;
		if ((b == vs) && isLineInCone(i, a, false))
			return false;

		// If the vertex with start and end point is collinear, (a vs) and (b, vs) is not in the polygon
		if (Line::isOnLine(a, b, vs)) {
			if (isLineInCone(i, a, false))
				return false;
			if (isLineInCone(i, b, false))
				return false;
		}
	}

	return true;
}

bool Polygon::isLineInCone(int startVertexIndex, const Vertex &endVertex, bool includeEdges) const {
	const Vertex &startVertex = vertices[startVertexIndex];
	const Vertex &nextVertex = vertices[(startVertexIndex + 1) % vertexCount];
	const Vertex &prevVertex = vertices[(startVertexIndex + vertexCount - 1) % vertexCount];

	if (Line::isVertexLeftOn(prevVertex, startVertex, nextVertex)) {
		if (includeEdges)
			return Line::isVertexLeftOn(endVertex, startVertex, nextVertex) &&
			       Line::isVertexLeftOn(startVertex, endVertex, prevVertex);
		else
			return Line::isVertexLeft(endVertex, startVertex, nextVertex) &&
			       Line::isVertexLeft(startVertex, endVertex, prevVertex);
	} else {
		if (includeEdges)
			return !(Line::isVertexLeft(endVertex, startVertex, prevVertex) &&
			         Line::isVertexLeft(startVertex, endVertex, nextVertex));
		else
			return !(Line::isVertexLeftOn(endVertex, startVertex, prevVertex) &&
			         Line::isVertexLeftOn(startVertex, endVertex, nextVertex));
	}
}

// Point-Polygon Tests
// -------------------

bool Polygon::isPointInPolygon(int x, int y, bool borderBelongsToPolygon) const {
	return isPointInPolygon(Vertex(x, y), borderBelongsToPolygon);
}

bool Polygon::isPointInPolygon(const Vertex &point, bool edgesBelongToPolygon) const {
	int rcross = 0; // Number of right-side overlaps
	int lcross = 0; // Number of left-side overlaps

	// Each edge is checked whether it cuts the outgoing stream from the point
	for (int i = 0; i < vertexCount; i++) {
		const Vertex &edgeStart = vertices[i];
		const Vertex &edgeEnd = vertices[(i + 1) % vertexCount];

		// A vertex is a point? Then it lies on one edge of the polygon
		if (point == edgeStart)
			return edgesBelongToPolygon;

		if ((edgeStart.y > point.y) != (edgeEnd.y > point.y)) {
			int term1 = (edgeStart.x - point.x) * (edgeEnd.y - point.y) - (edgeEnd.x - point.x) * (edgeStart.y - point.y);
			int term2 = (edgeEnd.y - point.y) - (edgeStart.y - edgeEnd.y);
			if ((term1 > 0) == (term2 >= 0))
				rcross++;
		}

		if ((edgeStart.y < point.y) != (edgeEnd.y < point.y)) {
			int term1 = (edgeStart.x - point.x) * (edgeEnd.y - point.y) - (edgeEnd.x - point.x) * (edgeStart.y - point.y);
			int term2 = (edgeEnd.y - point.y) - (edgeStart.y - edgeEnd.y);
			if ((term1 < 0) == (term2 <= 0))
				lcross++;
		}
	}

	// The point is on an adge, if the number of left and right intersections have the same even numbers
	if ((rcross % 2) != (lcross % 2))
		return edgesBelongToPolygon;

	// The point is strictly inside the polygon if and only if the number of overlaps is odd
	if ((rcross % 2) == 1)
		return true;
	else
		return false;
}

bool Polygon::persist(OutputPersistenceBlock &writer) {
	writer.write(vertexCount);
	for (int i = 0; i < vertexCount; ++i) {
		writer.write(vertices[i].x);
		writer.write(vertices[i].y);
	}

	return true;
}

bool Polygon::unpersist(InputPersistenceBlock &reader) {
	int storedvertexCount;
	reader.read(storedvertexCount);

	Common::Array<Vertex> storedvertices;
	for (int i = 0; i < storedvertexCount; ++i) {
		int x, y;
		reader.read(x);
		reader.read(y);
		storedvertices.push_back(Vertex(x, y));
	}

	init(storedvertexCount, &storedvertices[0]);

	return reader.isGood();
}

// Main Focus
// ----------

Vertex Polygon::getCentroid() const {
	return _centroid;
}

Vertex Polygon::computeCentroid() const {
	// Area of the polygon is calculated
	int doubleArea = 0;
	for (int i = 0; i < vertexCount; ++i) {
		doubleArea += vertices[i].x * vertices[i + 1].y - vertices[i + 1].x * vertices[i].y;
	}

	// Avoid division by zero in the next step
	if (doubleArea == 0)
		return Vertex();

	// Calculate centroid
	Vertex centroid;
	for (int i = 0; i < vertexCount; ++i) {
		int area = vertices[i].x * vertices[i + 1].y - vertices[i + 1].x * vertices[i].y;
		centroid.x += (vertices[i].x + vertices[i + 1].x) * area;
		centroid.y += (vertices[i].y + vertices[i + 1].y) * area;
	}
	centroid.x /= 3 * doubleArea;
	centroid.y /= 3 * doubleArea;

	return centroid;
}

} // End of namespace Sword25
