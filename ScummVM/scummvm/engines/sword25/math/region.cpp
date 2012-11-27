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

#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"

#include "sword25/math/region.h"
#include "sword25/math/walkregion.h"
#include "sword25/math/regionregistry.h"

namespace Sword25 {

Region::Region() : _valid(false), _type(RT_REGION) {
	RegionRegistry::instance().registerObject(this);
}

Region::Region(InputPersistenceBlock &reader, uint handle) : _valid(false), _type(RT_REGION) {
	RegionRegistry::instance().registerObject(this, handle);
	unpersist(reader);
}

uint Region::create(REGION_TYPE type) {
	Region *regionPtr = NULL;
	switch (type) {
	case RT_REGION:
		regionPtr = new Region();
		break;

	case RT_WALKREGION:
		regionPtr = new WalkRegion();
		break;

	default:
		assert(true);
	}

	return RegionRegistry::instance().resolvePtr(regionPtr);
}

uint Region::create(InputPersistenceBlock &reader, uint handle) {
	// Read type
	uint type;
	reader.read(type);

	// Depending on the type, create a new BS_Region or BS_WalkRegion object
	Region *regionPtr = NULL;
	if (type == RT_REGION) {
		regionPtr = new Region(reader, handle);
	} else if (type == RT_WALKREGION) {
		regionPtr = new WalkRegion(reader, handle);
	} else {
		assert(false);
	}

	return RegionRegistry::instance().resolvePtr(regionPtr);
}

Region::~Region() {
	RegionRegistry::instance().deregisterObject(this);
}

bool Region::init(const Polygon &contour, const Common::Array<Polygon> *pHoles) {
	// Reset object state
	_valid = false;
	_position = Vertex(0, 0);
	_polygons.clear();

	// Reserve sufficient  space for countour and holes in the polygon list
	if (pHoles)
		_polygons.reserve(1 + pHoles->size());
	else
		_polygons.reserve(1);

	// The first polygon will be the contour
	_polygons.push_back(Polygon());
	_polygons[0].init(contour.vertexCount, contour.vertices);
	// Make sure that the Vertecies in the Contour are arranged in a clockwise direction
	_polygons[0].ensureCWOrder();

	// Place the hole polygons in the following positions
	if (pHoles) {
		for (uint i = 0; i < pHoles->size(); ++i) {
			_polygons.push_back(Polygon());
			_polygons[i + 1].init((*pHoles)[i].vertexCount, (*pHoles)[i].vertices);
			_polygons[i + 1].ensureCWOrder();
		}
	}


	// Initialize bounding box
	updateBoundingBox();

	_valid = true;
	return true;
}

void Region::updateBoundingBox() {
	if (_polygons[0].vertexCount) {
		int minX = _polygons[0].vertices[0].x;
		int maxX = _polygons[0].vertices[0].x;
		int minY = _polygons[0].vertices[0].y;
		int maxY = _polygons[0].vertices[0].y;

		for (int i = 1; i < _polygons[0].vertexCount; i++) {
			if (_polygons[0].vertices[i].x < minX) minX = _polygons[0].vertices[i].x;
			else if (_polygons[0].vertices[i].x > maxX) maxX = _polygons[0].vertices[i].x;
			if (_polygons[0].vertices[i].y < minY) minY = _polygons[0].vertices[i].y;
			else if (_polygons[0].vertices[i].y > maxY) maxY = _polygons[0].vertices[i].y;
		}

		_boundingBox = Common::Rect(minX, minY, maxX + 1, maxY + 1);
	}
}

// Position Changes
void Region::setPos(int x, int y) {
	// Calculate the difference between the old and new position
	Vertex delta(x - _position.x, y - _position.y);

	// Save the new position
	_position = Vertex(x, y);

	// Move all the vertecies
	for (uint i = 0; i < _polygons.size(); ++i) {
		_polygons[i] += delta;
	}

	// Update the bounding box
	updateBoundingBox();
}

void Region::setPosX(int x) {
	setPos(x, _position.y);
}

void Region::setPosY(int y) {
	setPos(_position.x, y);
}

// Point-Region Tests
bool Region::isPointInRegion(int x, int y) const {
	// Test whether the point is in the bounding box
	if (_boundingBox.contains(x, y)) {
		// Test whether the point is in the contour
		if (_polygons[0].isPointInPolygon(x, y, true)) {
			// Test whether the point is in a hole
			for (uint i = 1; i < _polygons.size(); i++) {
				if (_polygons[i].isPointInPolygon(x, y, false))
					return false;
			}

			return true;
		}
	}

	return false;
}

bool Region::isPointInRegion(const Vertex &vertex) const {
	return isPointInRegion(vertex.x, vertex.y);
}

Vertex Region::findClosestRegionPoint(const Vertex &point) const {
	// Determine whether the point is inside a hole. If that is the case, the closest
	// point on the edge of the hole is determined
	int polygonIdx = 0;
	{
		for (uint i = 1; i < _polygons.size(); ++i) {
			if (_polygons[i].isPointInPolygon(point)) {
				polygonIdx = i;
				break;
			}
		}
	}

	const Polygon &polygon = _polygons[polygonIdx];

	assert(polygon.vertexCount > 1);

	// For each line of the polygon, calculate the point that is cloest to the given point
	// The point of this set with the smallest distance to the given point is the result.
	Vertex closestVertex = findClosestPointOnLine(polygon.vertices[0], polygon.vertices[1], point);
	int closestVertexDistance2 = closestVertex.distance(point);
	for (int i = 1; i < polygon.vertexCount; ++i) {
		int j = (i + 1) % polygon.vertexCount;

		Vertex curVertex = findClosestPointOnLine(polygon.vertices[i], polygon.vertices[j], point);
		if (curVertex.distance(point) < closestVertexDistance2) {
			closestVertex = curVertex;
			closestVertexDistance2 = curVertex.distance(point);
		}
	}

	// Determine whether the point is really within the region. This must not be so, as a result of rounding
	// errors can occur at the edge of polygons
	if (isPointInRegion(closestVertex))
		return closestVertex;
	else {
		// Try to construct a point within the region - 8 points are tested in the immediate vacinity
		// of the point
		if (isPointInRegion(closestVertex + Vertex(-2, -2)))
			return closestVertex + Vertex(-2, -2);
		else if (isPointInRegion(closestVertex + Vertex(0, -2)))
			return closestVertex + Vertex(0, -2);
		else if (isPointInRegion(closestVertex + Vertex(2, -2)))
			return closestVertex + Vertex(2, -2);
		else if (isPointInRegion(closestVertex + Vertex(-2, 0)))
			return closestVertex + Vertex(-2, 0);
		else if (isPointInRegion(closestVertex + Vertex(0, 2)))
			return closestVertex + Vertex(0, 2);
		else if (isPointInRegion(closestVertex + Vertex(-2, 2)))
			return closestVertex + Vertex(-2, 2);
		else if (isPointInRegion(closestVertex + Vertex(-2, 0)))
			return closestVertex + Vertex(2, 2);
		else if (isPointInRegion(closestVertex + Vertex(2, 2)))
			return closestVertex + Vertex(2, 2);

		// If no point could be found that way that lies within the region, find the next point
		closestVertex = polygon.vertices[0];
		int shortestVertexDistance2 = polygon.vertices[0].sqrDist(point);
		{
			for (int i = 1; i < polygon.vertexCount; i++) {
				int curDistance2 = polygon.vertices[i].sqrDist(point);
				if (curDistance2 < shortestVertexDistance2) {
					closestVertex = polygon.vertices[i];
					shortestVertexDistance2 = curDistance2;
				}
			}
		}

		warning("Clostest vertex forced because edgepoint was outside region.");
		return closestVertex;
	}
}

Vertex Region::findClosestPointOnLine(const Vertex &lineStart, const Vertex &lineEnd, const Vertex point) const {
	float vector1X = static_cast<float>(point.x - lineStart.x);
	float vector1Y = static_cast<float>(point.y - lineStart.y);
	float vector2X = static_cast<float>(lineEnd.x - lineStart.x);
	float vector2Y = static_cast<float>(lineEnd.y - lineStart.y);
	float vector2Length = sqrtf(vector2X * vector2X + vector2Y * vector2Y);
	vector2X /= vector2Length;
	vector2Y /= vector2Length;
	float distance = sqrtf(static_cast<float>((lineStart.x - lineEnd.x) * (lineStart.x - lineEnd.x) +
	                       (lineStart.y - lineEnd.y) * (lineStart.y - lineEnd.y)));
	float dot = vector1X * vector2X + vector1Y * vector2Y;

	if (dot <= 0)
		return lineStart;
	if (dot >= distance)
		return lineEnd;

	Vertex vector3(static_cast<int>(vector2X * dot + 0.5f), static_cast<int>(vector2Y * dot + 0.5f));
	return lineStart + vector3;
}

// Line of Sight
bool Region::isLineOfSight(const Vertex &a, const Vertex &b) const {
	assert(_polygons.size());

	// The line must be within the contour polygon, and outside of any hole polygons
	Common::Array<Polygon>::const_iterator iter = _polygons.begin();
	if (!(*iter).isLineInterior(a, b)) return false;
	for (iter++; iter != _polygons.end(); iter++)
		if (!(*iter).isLineExterior(a, b)) return false;

	return true;
}

// Persistence
bool Region::persist(OutputPersistenceBlock &writer) {
	bool Result = true;

	writer.write(static_cast<uint>(_type));
	writer.write(_valid);
	writer.write(_position.x);
	writer.write(_position.y);

	writer.write(_polygons.size());
	Common::Array<Polygon>::iterator It = _polygons.begin();
	while (It != _polygons.end()) {
		Result &= It->persist(writer);
		++It;
	}

	writer.write(_boundingBox.left);
	writer.write(_boundingBox.top);
	writer.write(_boundingBox.right);
	writer.write(_boundingBox.bottom);

	return Result;
}

bool Region::unpersist(InputPersistenceBlock &reader) {
	reader.read(_valid);
	reader.read(_position.x);
	reader.read(_position.y);

	_polygons.clear();
	uint PolygonCount;
	reader.read(PolygonCount);
	for (uint i = 0; i < PolygonCount; ++i) {
		_polygons.push_back(Polygon(reader));
	}

	reader.read(_boundingBox.left);
	reader.read(_boundingBox.top);
	reader.read(_boundingBox.right);
	reader.read(_boundingBox.bottom);

	return reader.isGood();
}

Vertex Region::getCentroid() const {
	if (_polygons.size() > 0)
		return _polygons[0].getCentroid();
	return
	    Vertex();
}

} // End of namespace Sword25
