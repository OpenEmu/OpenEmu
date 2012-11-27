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

#ifndef SWORD25_REGION_H
#define SWORD25_REGION_H

#include "sword25/kernel/common.h"
#include "sword25/kernel/persistable.h"
#include "sword25/math/vertex.h"
#include "sword25/math/polygon.h"
#include "common/rect.h"

namespace Sword25 {

/**
 * This class is the base class of all regions.
 *
 * The IsValid() method can be queried to see whether the object is in a valid state.
 * If this is not the case, the method Init() is the only method that may be invoked.
 * This class guarantees that the Vertecies outline of the hole, and the polygons are
 * arranged in a clockwise direction, so that the polygon working algorithms will
 * work properly.
 */
class Region : public Persistable {
protected:
	/**
	 * Creates a new BS_Region object
	 *
	 * After creation the object is invaild (IsValid() return false), but a call can
	 * be made later on to Init() to set up the region into a valid state.
	 */
	Region();

	Region(InputPersistenceBlock &reader, uint handle);

public:
	enum REGION_TYPE {
		RT_REGION,
		RT_WALKREGION
	};

	static uint create(REGION_TYPE type);
	static uint create(InputPersistenceBlock &reader, uint handle = 0);

	virtual ~Region();

	/**
	 * Initializes a BS_Region object
	 * @param Contour       A polygon indicating the outline of the region
	 * @param pHoles        A pointer to an array of polygons representing the hole state in the region.
	 * If the region has no holes, it must be passed as NULL. The default value is NULL.
	 * @return              Returns true if the initialisation was successful, otherwise false.
	 * @remark              If the region was already initialized, the old state will be deleted.
	 */
	virtual bool init(const Polygon &contour, const Common::Array<Polygon> *pHoles = NULL);

	//
	// Exploratory Methods
	//

	/**
	 * Specifies whether the object is in a valid state
	 * @return              Returns true if the object is in a valid state, otherwise false.
	 * @remark              Invalid objects can be made valid by calling Init with a valid state.
	 */
	bool isValid() const {
		return _valid;
	}

	/**
	 * Returns the position of the region
	 */
	const Vertex &getPosition() const {
		return _position;
	}

	/**
	 * Returns the X position of the region
	 */
	int getPosX() const {
		return _position.x;
	}

	/**
	 * Returns the Y position of the region
	 */
	int getPosY() const {
		return _position.y;
	}

	/**
	 * Indicates whether a point is inside the region
	 * @param Vertex        A verex with the co-ordinates of the test point
	 * @return              Returns true if the point is within the region, otherwise false.
	 */
	bool isPointInRegion(const Vertex &vertex) const;

	/**
	 * Indicates whether a point is inside the region
	 * @param X             The X position
	 * @param Y             The Y position
	 * @return              Returns true if the point is within the region, otherwise false.
	 */
	bool isPointInRegion(int x, int y) const;

	/**
	 * Returns the countour of the region
	 */
	const Polygon &getContour() const {
		return _polygons[0];
	}

	/**
	 * Returns the number of polygons in the hole region
	 */
	int getHoleCount() const {
		return static_cast<int>(_polygons.size() - 1);
	}

	/**
	 * Returns a specific hole polygon in the region
	 * @param i             The number of the hole to return.
	 * The index must be between 0 and GetHoleCount() - 1.
	 * @return              Returns the desired hole polygon
	 */
	inline const Polygon &getHole(uint i) const;

	/**
	 * For a point outside the region, finds the closest point inside the region
	 * @param Point         The point that is outside the region
	 * @return              Returns the point within the region which is closest
	 * @remark              This method does not always work with pixel accuracy.
	 * One should not therefore rely on the fact that there is really no point in
	 * the region which is closer to the given point.
	 */
	Vertex findClosestRegionPoint(const Vertex &point) const;

	/**
	 * Returns the centroid for the region
	 */
	Vertex getCentroid() const;

	bool isLineOfSight(const Vertex &a, const Vertex &b) const;

	//
	// Manipulation Methods
	//

	/**
	 * Sets the position of the region
	 * @param X             The new X psoition of the region
	 * @param Y             The new Y psoition of the region
	 */
	virtual void setPos(int x, int y);

	/**
	 * Sets the X position of the region
	 * @param X             The new X position of the region
	 */
	void setPosX(int x);

	/**
	 * Sets the Y position of the region
	 * @param Y             The new Y position of the region
	 */
	void setPosY(int y);

	//
	// Manipulation Methods
	//

	virtual bool persist(OutputPersistenceBlock &writer);
	virtual bool unpersist(InputPersistenceBlock &reader);

protected:
	/// This specifies the type of object
	REGION_TYPE _type;
	/// This variable indicates whether the current object state is valid
	bool _valid;
	/// This vertex is the position of the region
	Vertex _position;
	/// This array contains all the polygons that define the region. The first element of
	// the array is the contour, all others are the holes
	Common::Array<Polygon> _polygons;
	/// The bounding box for the region
	Common::Rect _boundingBox;

	/**
	 * Updates the bounding box of the region.
	 */
	void updateBoundingBox();

	/**
	 * Find the point on a line which is closest to another point
	 * @param LineStart     The start of the line
	 * @param LineEnd       The end of the line
	 * @param Point         The point to be compared against
	 * @return              Returns the point on the line which is cloest to the passed point.
	 */
	Vertex findClosestPointOnLine(const Vertex &lineStart, const Vertex &lineEnd, const Vertex point) const;
};

inline const Polygon &Region::getHole(uint i) const {
	assert(i < _polygons.size() - 1);
	return _polygons[i + 1];
}

} // End of namespace Sword25

#endif
