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

#include "tony/utils.h"
#include "tony/tony.h"
#include "tony/mpal/lzo.h"

namespace Tony {

/**
 * Extracts a string from a data stream
 * @param df                data stream
 */
Common::String readString(Common::ReadStream &df) {
	Common::String var;
	uint8 len = df.readByte();

	for (int i = 0; i < len; i++) {
		char c;
		c = df.readByte();
		var += c;
	}

	return var;
}

/****************************************************************************\
*       RMPoint methods
\****************************************************************************/

/**
 * Constructor
 */
RMPoint::RMPoint() {
	_x = _y = 0;
}

/**
 * Copy constructor
 */
RMPoint::RMPoint(const RMPoint &p) {
	_x = p._x;
	_y = p._y;
}

/**
 * Constructor with integer parameters
 */
RMPoint::RMPoint(int x1, int y1) {
	_x = x1;
	_y = y1;
}

/**
 * Copy operator
 */
RMPoint &RMPoint::operator=(RMPoint p) {
	_x = p._x;
	_y = p._y;

	return *this;
}

/**
 * Set a point
 */
void RMPoint::set(int x1, int y1) {
	_x = x1;
	_y = y1;
}

/**
 * Offsets the point by another point
 */
void RMPoint::offset(const RMPoint &p) {
	_x += p._x;
	_y += p._y;
}

/**
 * Offsets the point by a specified offset
 */
void RMPoint::offset(int xOff, int yOff) {
	_x += xOff;
	_y += yOff;
}

/**
 * Sums together two points
 */
RMPoint operator+(RMPoint p1, RMPoint p2) {
	RMPoint p(p1);

	return (p += p2);
}

/**
 * Subtracts two points
 */
RMPoint operator-(RMPoint p1, RMPoint p2) {
	RMPoint p(p1);

	return (p -= p2);
}

/**
 * Sum (offset) of a point
 */
RMPoint &RMPoint::operator+=(RMPoint p) {
	offset(p);
	return *this;
}

/**
 * Subtract (offset) of a point
 */
RMPoint &RMPoint::operator-=(RMPoint p) {
	offset(-p);
	return *this;
}

/**
 * Inverts a point
 */
RMPoint RMPoint::operator-() {
	RMPoint p;

	p._x = -_x;
	p._y = -_y;

	return p;
}

/**
 * Equality operator
 */
bool RMPoint::operator==(RMPoint p) {
	return ((_x == p._x) && (_y == p._y));
}

/**
 * Not equal operator
 */
bool RMPoint::operator!=(RMPoint p) {
	return ((_x != p._x) || (_y != p._y));
}

/**
 * Reads a point from a stream
 */
void RMPoint::readFromStream(Common::ReadStream &ds) {
	_x = ds.readSint32LE();
	_y = ds.readSint32LE();
}

/****************************************************************************\
*       RMPointReference methods
\****************************************************************************/

RMPointReference::RMPointReference(int &x, int &y): _x(x), _y(y) {
}

RMPointReference &RMPointReference::operator=(const RMPoint &p) {
	_x = p._x; _y = p._y;
	return *this;
}

RMPointReference &RMPointReference::operator-=(const RMPoint &p) {
	_x -= p._x; _y -= p._y;
	return *this;
}

RMPointReference::operator RMPoint() const {
	return RMPoint(_x, _y);
}

/****************************************************************************\
*       RMRect methods
\****************************************************************************/

RMRect::RMRect(): _topLeft(_x1, _y1), _bottomRight(_x2, _y2) {
	setEmpty();
}

void RMRect::setEmpty() {
	_x1 = _y1 = _x2 = _y2 = 0;
}

RMRect::RMRect(const RMPoint &p1, const RMPoint &p2): _topLeft(_x1, _y1), _bottomRight(_x2, _y2) {
	setRect(p1, p2);
}

RMRect::RMRect(int X1, int Y1, int X2, int Y2): _topLeft(_x1, _y1), _bottomRight(_x2, _y2) {
	setRect(X1, Y1, X2, Y2);
}

RMRect::RMRect(const RMRect &rc): _topLeft(_x1, _y1), _bottomRight(_x2, _y2) {
	copyRect(rc);
}

void RMRect::setRect(const RMPoint &p1, const RMPoint &p2) {
	_x1 = p1._x;
	_y1 = p1._y;
	_x2 = p2._x;
	_y2 = p2._y;
}

void RMRect::setRect(int X1, int Y1, int X2, int Y2) {
	_x1 = X1;
	_y1 = Y1;
	_x2 = X2;
	_y2 = Y2;
}

void RMRect::setRect(const RMRect &rc) {
	copyRect(rc);
}

void RMRect::copyRect(const RMRect &rc) {
	_x1 = rc._x1;
	_y1 = rc._y1;
	_x2 = rc._x2;
	_y2 = rc._y2;
}

RMPointReference &RMRect::topLeft() {
	return _topLeft;
}

RMPointReference &RMRect::bottomRight() {
	return _bottomRight;
}

RMPoint RMRect::center() {
	return RMPoint((_x2 - _x1) / 2, (_y2 - _y1) / 2);
}

int RMRect::width() const {
	return _x2 - _x1;
}

int RMRect::height() const {
	return _y2 - _y1;
}

int RMRect::size() const {
	return width() * height();
}

RMRect::operator Common::Rect() const {
	return Common::Rect(_x1, _y1, _x2, _y2);
}

bool RMRect::isEmpty() const {
	return (_x1 == 0 && _y1 == 0 && _x2 == 0 && _y2 == 0);
}

const RMRect &RMRect::operator=(const RMRect &rc) {
	copyRect(rc);
	return *this;
}

void RMRect::offset(int xOff, int yOff) {
	_x1 += xOff;
	_y1 += yOff;
	_x2 += xOff;
	_y2 += yOff;
}

void RMRect::offset(const RMPoint &p) {
	_x1 += p._x;
	_y1 += p._y;
	_x2 += p._x;
	_y2 += p._y;
}

const RMRect &RMRect::operator+=(RMPoint p) {
	offset(p);
	return *this;
}

const RMRect &RMRect::operator-=(RMPoint p) {
	offset(-p);
	return *this;
}

RMRect operator+(const RMRect &rc, RMPoint p) {
	RMRect r(rc);
	return (r += p);
}

RMRect operator-(const RMRect &rc, RMPoint p) {
	RMRect r(rc);

	return (r -= p);
}

RMRect operator+(RMPoint p, const RMRect &rc) {
	RMRect r(rc);

	return (r += p);
}

RMRect operator-(RMPoint p, const RMRect &rc) {
	RMRect r(rc);

	return (r += p);
}

bool RMRect::operator==(const RMRect &rc) {
	return ((_x1 == rc._x1) && (_y1 == rc._y1) && (_x2 == rc._x2) && (_y2 == rc._y2));
}

bool RMRect::operator!=(const RMRect &rc) {
	return ((_x1 != rc._x1) || (_y1 != rc._y1) || (_x2 != rc._x2) || (_y2 != rc._y2));
}

void RMRect::normalizeRect() {
	setRect(MIN(_x1, _x2), MIN(_y1, _y2), MAX(_x1, _x2), MAX(_y1, _y2));
}

void RMRect::readFromStream(Common::ReadStream &ds) {
	_x1 = ds.readSint32LE();
	_y1 = ds.readSint32LE();
	_x2 = ds.readSint32LE();
	_y2 = ds.readSint32LE();
}

/**
 * Check if RMPoint is in RMRect
 */
bool RMRect::ptInRect(const RMPoint &pt) {
	return (pt._x >= _x1 && pt._x <= _x2 && pt._y >= _y1 && pt._y <= _y2);
}

/****************************************************************************\
*       Resource Update
\****************************************************************************/

RMResUpdate::RMResUpdate() {
	_infos = NULL;
	_numUpd = 0;
}

RMResUpdate::~RMResUpdate() {
	if (_infos) {
		delete[] _infos;
		_infos = NULL;
	}

	if (_hFile.isOpen())
		_hFile.close();
}

void RMResUpdate::init(const Common::String &fileName) {
	// Open the resource update file
	if (!_hFile.open(fileName))
		// It doesn't exist, so exit immediately
		return;

	_hFile.readByte(); // Version, unused

	_numUpd = _hFile.readUint32LE();

	_infos = new ResUpdInfo[_numUpd];

	// Load the index of the resources in the file
	for (uint32 i = 0; i < _numUpd; ++i) {
		ResUpdInfo &info = _infos[i];

		info._dwRes = _hFile.readUint32LE();
		info._offset = _hFile.readUint32LE();
		info._size = _hFile.readUint32LE();
		info._cmpSize = _hFile.readUint32LE();
	}
}

MpalHandle RMResUpdate::queryResource(uint32 dwRes) {
	// If there isn't an update file, return NULL
	if (!_hFile.isOpen())
		return NULL;

	uint32 i;
	for (i = 0; i < _numUpd; ++i)
		if (_infos[i]._dwRes == dwRes)
			// Found the index
			break;

	if (i == _numUpd)
		// Couldn't find a matching resource, so return NULL
		return NULL;

	const ResUpdInfo &info = _infos[i];
	byte *cmpBuf = new byte[info._cmpSize];
	uint32 dwRead;

	// Move to the correct offset and read in the compressed data
	_hFile.seek(info._offset);
	dwRead = _hFile.read(cmpBuf, info._cmpSize);

	if (info._cmpSize > dwRead) {
		// Error occurred reading data, so return NULL
		delete[] cmpBuf;
		return NULL;
	}

	// Allocate space for the output resource
	MpalHandle destBuf = globalAllocate(0, info._size);
	byte *lpDestBuf = (byte *)globalLock(destBuf);
	uint32 dwSize;

	// Decompress the data
	lzo1x_decompress(cmpBuf, info._cmpSize, lpDestBuf, &dwSize);

	// Delete buffer for compressed data
	delete [] cmpBuf;

	// Return the resource
	globalUnlock(destBuf);
	return destBuf;
}

} // End of namespace Tony
