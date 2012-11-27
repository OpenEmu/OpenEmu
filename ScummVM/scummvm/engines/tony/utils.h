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
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#ifndef TONY_UTILS_H
#define TONY_UTILS_H

#include "common/scummsys.h"
#include "common/file.h"
#include "common/rect.h"
#include "common/str.h"
#include "tony/mpal/memory.h"

namespace Tony {

using namespace ::Tony::MPAL;

Common::String readString(Common::ReadStream &ds);

/**
 * Point class
 */
class RMPoint {
public:
	int _x, _y;

public:
	// Constructor
	RMPoint();
	RMPoint(const RMPoint &p);
	RMPoint(int x1, int y1);

	// Copy
	RMPoint &operator=(RMPoint p);

	// Set
	void set(int x1, int y1);

	// Offset
	void offset(int xOff, int yOff);
	void offset(const RMPoint &p);
	friend RMPoint operator+(RMPoint p1, RMPoint p2);
	friend RMPoint operator-(RMPoint p1, RMPoint p2);
	RMPoint &operator+=(RMPoint p);
	RMPoint &operator-=(RMPoint p);
	RMPoint operator-();

	// Comparison
	bool operator==(RMPoint p);
	bool operator!=(RMPoint p);

	// Casting a POINT
	operator Common::Point() const;

	// Extraction from data streams
	void readFromStream(Common::ReadStream &ds);
};

class RMPointReference {
public:
	int &_x;
	int &_y;

	RMPointReference(int &x, int &y);
	RMPointReference &operator=(const RMPoint &p);
	RMPointReference &operator-=(const RMPoint &p);
	operator RMPoint() const;
};

class RMRect {
public:
	int _x1, _y1;
	int _x2, _y2;
	RMPointReference _topLeft;
	RMPointReference _bottomRight;

public:
	RMRect();
	RMRect(int x1, int y1, int x2, int y2);
	RMRect(const RMPoint &p1, const RMPoint &p2);
	RMRect(const RMRect &rc);

	// Attributes
	RMPointReference &topLeft();
	RMPointReference &bottomRight();
	RMPoint center();
	int width() const;
	int height() const;
	bool isEmpty() const;
	int size() const;
	operator Common::Rect() const;

	// Set
	void setRect(int x1, int y1, int x2, int y2);
	void setRect(const RMPoint &p1, const RMPoint &p2);
	void setEmpty();

	// Copiers
	void setRect(const RMRect &rc);
	void copyRect(const RMRect &rc);
	const RMRect &operator=(const RMRect &rc);

	// Offset
	void offset(int xOff, int yOff);
	void offset(const RMPoint &p);
	friend RMRect operator+(const RMRect &rc, RMPoint p);
	friend RMRect operator-(const RMRect &rc, RMPoint p);
	friend RMRect operator+(RMPoint p, const RMRect &rc);
	friend RMRect operator-(RMPoint p, const RMRect &rc);
	const RMRect &operator+=(RMPoint p);
	const RMRect &operator-=(RMPoint p);

	// Comparison
	bool operator==(const RMRect &rc);
	bool operator!=(const RMRect &rc);

	// Normalize
	void normalizeRect();

	// Point in rect
	bool ptInRect(const RMPoint &pt);

	// Extract from data stream
	void readFromStream(Common::ReadStream &ds);
};

/**
 * Resource update manager
 */
class RMResUpdate {
	struct ResUpdInfo {
		uint32 _dwRes;
		uint32 _offset;
		uint32 _size;
		uint32 _cmpSize;
	};

	uint32 _numUpd;
	ResUpdInfo *_infos;
	Common::File _hFile;

public:
	RMResUpdate();
	~RMResUpdate();

	void init(const Common::String &fileName);
	MpalHandle queryResource(uint32 dwRes);
};

} // End of namespace Tony

#endif /* TONY_H */
