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

#ifndef SWORD25_WALKREGION_H
#define SWORD25_WALKREGION_H

#include "common/array.h"
#include "sword25/kernel/common.h"
#include "sword25/math/region.h"

namespace Sword25 {

typedef Common::Array<Vertex> BS_Path;

/**
 * This class represents the region in which the main character can move
 */
class WalkRegion : public Region {
	friend class Region;

protected:
	WalkRegion();
	WalkRegion(InputPersistenceBlock &Reader, uint handle);

public:
	virtual ~WalkRegion();

	virtual bool init(const Polygon &contour, const Common::Array<Polygon> *pHoles = 0);

	/**
	 * Get the shortest path between two points in the region
	 *
	 * This method requires that the starting point lies within the region. The end point
	 * may lie outside the region. Int his case, the end is chosen as the cloest point to it
	 * that lies within the region.
	 *
	 * @param X1            X Co-ordinate of the start point
	 * @param Y1            Y Co-ordinate of the start point
	 * @param X2            X Co-ordinate of the end point
	 * @param Y2            Y Co-ordinate of the end point
	 * @param Path          An empty BS_Path that will be set to the resulting path
	 * @return              Returns false if the result is invalid, otherwise returns true.
	 */
	bool queryPath(int x1, int y1, int x2, int y2, BS_Path &path) {
		return queryPath(Vertex(x1, y1), Vertex(x2, y2), path);
	}

	/**
	 * Get the shortest path between two points in the region.
	 *
	 * @param StartPoint    The start point
	 * @param EndPoint      The end point
	 * @param Path          An empty BS_Path that will be set to the resulting path
	 * @return              Returns false if the result is invalid, otherwise returns true.
	*/
	bool queryPath(Vertex startPoint, Vertex endPoint, BS_Path &path);

	virtual void setPos(int x, int y);

	const Common::Array<Vertex> &getNodes() const {
		return _nodes;
	}
	const Common::Array< Common::Array<int> > &getVisibilityMatrix() const {
		return _visibilityMatrix;
	}

	virtual bool persist(OutputPersistenceBlock &writer);
	virtual bool unpersist(InputPersistenceBlock &reader);

private:
	Common::Array<Vertex> _nodes;
	Common::Array< Common::Array<int> > _visibilityMatrix;

	void initNodeVector();
	void computeVisibilityMatrix();
	bool checkAndPrepareStartAndEnd(Vertex &start, Vertex &end) const;
	bool findPath(const Vertex &start, const Vertex &end, BS_Path &path) const;
};

} // End of namespace Sword25

#endif
