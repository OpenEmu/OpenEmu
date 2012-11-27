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

#include "sword25/kernel/kernel.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/math/walkregion.h"
#include "sword25/math/line.h"

namespace Sword25 {

static const int Infinity = 0x7fffffff;

WalkRegion::WalkRegion() {
	_type = RT_WALKREGION;
}

WalkRegion::WalkRegion(InputPersistenceBlock &reader, uint handle) :
	Region(reader, handle) {
	_type = RT_WALKREGION;
	unpersist(reader);
}

WalkRegion::~WalkRegion() {
}

bool WalkRegion::init(const Polygon &contour, const Common::Array<Polygon> *pHoles) {
	// Default initialisation of the region
	if (!Region::init(contour, pHoles)) return false;

	// Prepare structures for pathfinding
	initNodeVector();
	computeVisibilityMatrix();

	// Signal success
	return true;
}

bool WalkRegion::queryPath(Vertex startPoint, Vertex endPoint, BS_Path &path) {
	assert(path.empty());

	// If the start and finish are identical, no path can be found trivially
	if (startPoint == endPoint)
		return true;

	// Ensure that the start and finish are valid and find new start points if either
	// are outside the polygon
	if (!checkAndPrepareStartAndEnd(startPoint, endPoint)) return false;

	// If between the start and point a line of sight exists, then it can be returned.
	if (isLineOfSight(startPoint, endPoint)) {
		path.push_back(startPoint);
		path.push_back(endPoint);
		return true;
	}

	return findPath(startPoint, endPoint, path);
}

struct DijkstraNode {
	typedef Common::Array<DijkstraNode> Container;
	typedef Container::iterator Iter;
	typedef Container::const_iterator ConstIter;

	DijkstraNode() : parentIter(), cost(Infinity), chosen(false) {}
	ConstIter   parentIter;
	int         cost;
	bool        chosen;
};

static void initDijkstraNodes(DijkstraNode::Container &dijkstraNodes, const Region &region,
                              const Vertex &start, const Common::Array<Vertex> &nodes) {
	// Allocate sufficient space in the array
	dijkstraNodes.resize(nodes.size());

	// Initialize all the nodes which are visible from the starting node
	DijkstraNode::Iter dijkstraIter = dijkstraNodes.begin();
	for (Common::Array<Vertex>::const_iterator nodesIter = nodes.begin();
	        nodesIter != nodes.end(); nodesIter++, dijkstraIter++) {
		(*dijkstraIter).parentIter = dijkstraNodes.end();
		if (region.isLineOfSight(*nodesIter, start))(*dijkstraIter).cost = (*nodesIter).distance(start);
	}
	assert(dijkstraIter == dijkstraNodes.end());
}

static DijkstraNode::Iter chooseClosestNode(DijkstraNode::Container &nodes) {
	DijkstraNode::Iter closestNodeInter = nodes.end();
	int minCost = Infinity;

	for (DijkstraNode::Iter iter = nodes.begin(); iter != nodes.end(); iter++) {
		if (!(*iter).chosen && (*iter).cost < minCost) {
			minCost = (*iter).cost;
			closestNodeInter = iter;
		}
	}

	return closestNodeInter;
}

static void relaxNodes(DijkstraNode::Container &nodes,
                       const Common::Array< Common::Array<int> > &visibilityMatrix,
                       const DijkstraNode::ConstIter &curNodeIter) {
	// All the successors of the current node that have not been chosen will be
	// inserted into the boundary node list, and the cost will be updated if
	// a shorter path has been found to them.

	int curNodeIndex = curNodeIter - nodes.begin();
	for (uint i = 0; i < nodes.size(); i++) {
		int cost = visibilityMatrix[curNodeIndex][i];
		if (!nodes[i].chosen && cost != Infinity) {
			int totalCost = (*curNodeIter).cost + cost;
			if (totalCost < nodes[i].cost) {
				nodes[i].parentIter = curNodeIter;
				nodes[i].cost = totalCost;
			}
		}
	}
}

static void relaxEndPoint(const Vertex &curNodePos,
                          const DijkstraNode::ConstIter &curNodeIter,
                          const Vertex &endPointPos,
                          DijkstraNode &endPoint,
                          const Region &region) {
	if (region.isLineOfSight(curNodePos, endPointPos)) {
		int totalCost = (*curNodeIter).cost + curNodePos.distance(endPointPos);
		if (totalCost < endPoint.cost) {
			endPoint.parentIter = curNodeIter;
			endPoint.cost = totalCost;
		}
	}
}

template<class T>
void reverseArray(Common::Array<T> &arr) {
	const uint size = arr.size();
	if (size < 2)
		return;

	for (uint i = 0; i <= (size / 2 - 1); ++i) {
		SWAP(arr[i], arr[size - i - 1]);
	}
}

bool WalkRegion::findPath(const Vertex &start, const Vertex &end, BS_Path &path) const {
	// This is an implementation of Dijkstra's algorithm

	// Initialize edge node list
	DijkstraNode::Container dijkstraNodes;
	initDijkstraNodes(dijkstraNodes, *this, start, _nodes);

	// The end point is treated separately, since it does not exist in the visibility graph
	DijkstraNode endPoint;

	// Since a node is selected each round from the node list, and can never be selected again
	// after that, the maximum number of loop iterations is limited by the number of nodes
	for (uint i = 0; i < _nodes.size(); i++) {
		// Determine the nearest edge node in the node list
		DijkstraNode::Iter nodeInter = chooseClosestNode(dijkstraNodes);

		// If no free nodes are absent from the edge node list, there is no path from start
		// to end node. This case should never occur, since the number of loop passes is
		// limited, but etter safe than sorry
		if (nodeInter == dijkstraNodes.end())
			return false;

		// If the destination point is closer than the point cost, scan can stop
		(*nodeInter).chosen = true;
		if (endPoint.cost <= (*nodeInter).cost) {
			// Insert the end point in the list
			path.push_back(end);

			// The list is done in reverse order and inserted into the path
			DijkstraNode::ConstIter curNode = endPoint.parentIter;
			while (curNode != dijkstraNodes.end()) {
				assert((*curNode).chosen);
				path.push_back(_nodes[curNode - dijkstraNodes.begin()]);
				curNode = (*curNode).parentIter;
			}

			// The starting point is inserted into the path
			path.push_back(start);

			// The nodes of the path must be untwisted, as they were extracted in reverse order.
			// This step could be saved if the path from end to the beginning was desired
			reverseArray<Vertex>(path);

			return true;
		}

		// Relaxation step for nodes of the graph, and perform the end nodes
		relaxNodes(dijkstraNodes, _visibilityMatrix, nodeInter);
		relaxEndPoint(_nodes[nodeInter - dijkstraNodes.begin()], nodeInter, end, endPoint, *this);
	}

	// If the loop has been completely run through, all the nodes have been chosen, and still
	// no path was found. There is therefore no path available
	return false;
}

void WalkRegion::initNodeVector() {
	// Empty the Node list
	_nodes.clear();

	// Determine the number of nodes
	int nodeCount = 0;
	{
		for (uint i = 0; i < _polygons.size(); i++)
			nodeCount += _polygons[i].vertexCount;
	}

	// Knoten-Vector füllen
	_nodes.reserve(nodeCount);
	{
		for (uint j = 0; j < _polygons.size(); j++)
			for (int i = 0; i < _polygons[j].vertexCount; i++)
				_nodes.push_back(_polygons[j].vertices[i]);
	}
}

void WalkRegion::computeVisibilityMatrix() {
	// Initialize visibility matrix
	_visibilityMatrix = Common::Array< Common::Array <int> >();
	for (uint idx = 0; idx < _nodes.size(); ++idx) {
		Common::Array<int> arr;
		for (uint idx2 = 0; idx2 < _nodes.size(); ++idx2)
			arr.push_back(Infinity);

		_visibilityMatrix.push_back(arr);
	}

	// Calculate visibility been vertecies
	for (uint j = 0; j < _nodes.size(); ++j) {
		for (uint i = j; i < _nodes.size(); ++i)   {
			if (isLineOfSight(_nodes[i], _nodes[j])) {
				// There is a line of sight, so save the distance between the two
				int distance = _nodes[i].distance(_nodes[j]);
				_visibilityMatrix[i][j] = distance;
				_visibilityMatrix[j][i] = distance;
			} else {
				// There is no line of sight, so save Infinity as the distance
				_visibilityMatrix[i][j] = Infinity;
				_visibilityMatrix[j][i] = Infinity;
			}
		}
	}
}

bool WalkRegion::checkAndPrepareStartAndEnd(Vertex &start, Vertex &end) const {
	if (!isPointInRegion(start)) {
		Vertex newStart = findClosestRegionPoint(start);

		// Check to make sure the point is really in the region. If not, stop with an error
		if (!isPointInRegion(newStart)) {
			error("Constructed startpoint ((%d,%d) from (%d,%d)) is not inside the region.",
			               newStart.x, newStart.y,
			               start.x, start.y);
			return false;
		}

		start = newStart;
	}

	// If the destination is outside the region, a point is determined that is within the region,
	// and that is used as an endpoint instead
	if (!isPointInRegion(end)) {
		Vertex newEnd = findClosestRegionPoint(end);

		// Make sure that the determined point is really within the region
		if (!isPointInRegion(newEnd)) {
			error("Constructed endpoint ((%d,%d) from (%d,%d)) is not inside the region.",
			               newEnd.x, newEnd.y,
			               end.x, end.y);
			return false;
		}

		end = newEnd;
	}

	// Signal success
	return true;
}

void WalkRegion::setPos(int x, int y) {
	// Calculate the difference between old and new position
	Vertex Delta(x - _position.x, y - _position.y);

	// Move all the nodes
	for (uint i = 0; i < _nodes.size(); i++)
		_nodes[i] += Delta;

	// Move regions
	Region::setPos(x, y);
}

bool WalkRegion::persist(OutputPersistenceBlock &writer) {
	bool result = true;

	// Persist the parent region
	result &= Region::persist(writer);

	// Persist the nodes
	writer.write(_nodes.size());
	Common::Array<Vertex>::const_iterator it = _nodes.begin();
	while (it != _nodes.end()) {
		writer.write(it->x);
		writer.write(it->y);
		++it;
	}

	// Persist the visibility matrix
	writer.write(_visibilityMatrix.size());
	Common::Array< Common::Array<int> >::const_iterator rowIter = _visibilityMatrix.begin();
	while (rowIter != _visibilityMatrix.end()) {
		writer.write(rowIter->size());
		Common::Array<int>::const_iterator colIter = rowIter->begin();
		while (colIter != rowIter->end()) {
			writer.write(*colIter);
			++colIter;
		}

		++rowIter;
	}

	return result;
}

bool WalkRegion::unpersist(InputPersistenceBlock &reader) {
	bool result = true;

	// The parent object was already loaded in the constructor of BS_Region, so at
	// this point only the additional data from BS_WalkRegion needs to be loaded

	// Node load
	uint nodeCount;
	reader.read(nodeCount);
	_nodes.clear();
	_nodes.resize(nodeCount);
	Common::Array<Vertex>::iterator it = _nodes.begin();
	while (it != _nodes.end()) {
		reader.read(it->x);
		reader.read(it->y);
		++it;
	}

	// Visibility matrix load
	uint rowCount;
	reader.read(rowCount);
	_visibilityMatrix.clear();
	_visibilityMatrix.resize(rowCount);
	Common::Array< Common::Array<int> >::iterator rowIter = _visibilityMatrix.begin();
	while (rowIter != _visibilityMatrix.end()) {
		uint colCount;
		reader.read(colCount);
		rowIter->resize(colCount);
		Common::Array<int>::iterator colIter = rowIter->begin();
		while (colIter != rowIter->end()) {
			reader.read(*colIter);
			++colIter;
		}

		++rowIter;
	}

	return result && reader.isGood();
}

} // End of namespace Sword25
