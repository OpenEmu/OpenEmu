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

#ifndef SWORD25_POLYGON_H
#define SWORD25_POLYGON_H

// Includes
#include "sword25/kernel/common.h"
#include "sword25/kernel/persistable.h"
#include "sword25/math/vertex.h"

namespace Sword25 {

class Vertex;

/**
    @brief Eine Polygonklasse.
*/
class Polygon : public Persistable {
public:
	/**
	 * Creates an object of type #BS_Polygon, containing 0 Vertecies.
	 *
	 * With the method Init(), Vertices can be added in later
	 */
	Polygon();

	/**
	 * Copy constructor
	 */
	Polygon(const Polygon &other);

	/**
	 * Creates a polygon using persisted data
	 */
	Polygon(InputPersistenceBlock &reader);

	/**
	 * Creaes an object of type #BS_Polygon, and assigns Vertices to it
	 * @param VertexCount       The number of vertices being passed
	 * @param Vertecies         An array of BS_Vertex objects representing the vertices in the polygon.
	 * @remark                  The Vertecies that define a polygon must not have any self-intersections.
	 * If the polygon does have self-intersections, then an empty polygon object is created.
	 */
	Polygon(int vertexCount_, const Vertex *vertices_);

	/**
	 * Deletes the BS_Polygon object
	 */
	virtual ~Polygon();

	/**
	 * Initializes the BS_Polygon with a list of Vertecies.
	 *
	 * The Vertices need to define a polygon must not have self-intersections.
	 * If a polygon already has verticies, this will re-initialize it with the new list.
	 *
	 * @param VertexCount       The number of vertices being passed
	 * @param Vertecies         An array of BS_Vertex objects representing the vertices in the polygon.
	 * @return                  Returns false if the Vertecies have self-intersections. In this case,
	 * the object is not initialized.
	 */
	bool init(int vertexCount_, const Vertex *vertices_);

	//
	// ** Exploratory methods **
	//

	/**
	 * Checks whether the Vertecies of the polygon are arranged in a clockwise direction.
	 * @return                  Returns true if the Vertecies of the polygon are arranged clockwise or co-planar.
	 * Returns false if the Vertecies of the polygon are arrange counter-clockwise.
	 * @remark                  This method only returns a meaningful result if the polygon has at least three Vertecies.
	 */
	bool isCW() const;

	/**
	 * Checks whether the Vertices of the polygon are arranged in a counter-clockwise direction.
	 * @return                  Returns true if the Vertecies of the polygon are arranged counter-clockwise.
	 * Returns false if the Vertecies of the polygon are arranged clockwise or co-planar.
	 * @remark                  This method only returns a meaningful result if the polygon has at least three Vertecies.
	 */
	bool isCCW() const;

	/**
	 * Checks whether a point is inside the polygon
	 * @param Vertex            A Vertex with the co-ordinates of the point to be tested.
	 * @param BorderBelongsToPolygon    Specifies whether the edge of the polygon should be considered
	 * @return                  Returns true if the point is inside the polygon, false if it is outside.
	 */
	bool isPointInPolygon(const Vertex &vertex, bool borderBelongsToPolygon = true) const;

	/**
	 * Checks whether a point is inside the polygon
	 * @param X                 The X position of the point
	 * @param Y                 The Y position of the point
	 * @param BorderBelongsToPolygon    Specifies whether the edge of the polygon should be considered
	 * @return                  Returns true if the point is inside the polygon, false if it is outside.
	 */
	bool isPointInPolygon(int x, int y, bool borderBelongsToPolygon = true) const;

	/**
	 * Returns the focus/centroid of the polygon
	 */
	Vertex getCentroid() const;

	// Edge belongs to the polygon
	// Polygon must be CW
	bool isLineInterior(const Vertex &a, const Vertex &b) const;
	// Edge does not belong to the polygon
	// Polygon must be CW
	bool isLineExterior(const Vertex &a, const Vertex &b) const;

	//
	// Manipulation methods
	//

	/**
	 * Ensures that the Vertecies of the polygon are arranged in a clockwise direction
	 */
	void ensureCWOrder();

	/**
	 * Ensures that the Vertecies of the polygon are arranged in a counter-clockwise direction
	 */
	void ensureCCWOrder();

	/**
	 * Reverses the Vertecies order.
	 */
	void reverseVertexOrder();

	/**
	 * Moves the polygon.
	 * @param Delta             The vertex around the polygon to be moved.
	 */
	void operator+=(const Vertex &delta);

	//
	//------------------
	//

	/// Specifies the number of Vertecies in the Vertecies array.
	int vertexCount;
	/// COntains the Vertecies of the polygon
	Vertex *vertices;

	virtual bool persist(OutputPersistenceBlock &writer);
	virtual bool unpersist(InputPersistenceBlock &reader);

	Polygon &operator=(const Polygon &p) {
		init(p.vertexCount, p.vertices);
		return *this;
	}

private:
	bool _isCW;
	Vertex _centroid;

	/**
	 * Computes the centroid of the polygon.
	 */
	Vertex computeCentroid() const;

	/**
	 * Determines how the Vertecies of the polygon are arranged.
	 * @return                  Returns true if the Vertecies are arranged in a clockwise
	 * direction, otherwise false.
	 */
	bool computeIsCW() const;

	/**
	 * Calculates the cross product of three Vertecies
	 * @param V1                The first Vertex
	 * @param V2                The second Vertex
	 * @param V3                The third Vertex
	 * @return                  Returns the cross-product of the three vertecies
	 * @todo                    This method would be better as a method of the BS_Vertex class
	 */
	int crossProduct(const Vertex &v1, const Vertex &v2, const Vertex &v3) const;

	/**
	 * Checks whether the polygon is self-intersecting
	 * @return                  Returns true if the polygon is self-intersecting.
	 * Returns false if the polygon is not self-intersecting.
	 */
	bool checkForSelfIntersection() const;

	/**
	 * Find the vertex of the polygon that is located below the right-most point,
	 * and returns it's index in the vertex array.
	 * @return                  Returns the index of the vertex at the bottom-right of the polygon.
	 * Returns -1 if the vertex list is empty.
	 */
	int findLRVertexIndex() const;

	bool isLineInCone(int startVertexIndex, const Vertex &endVertex, bool includeEdges) const;
};

} // End of namespace Sword25

#endif
