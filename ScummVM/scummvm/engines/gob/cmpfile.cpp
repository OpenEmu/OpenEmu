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

#include "common/stream.h"
#include "common/substream.h"
#include "common/str.h"

#include "gob/gob.h"
#include "gob/util.h"
#include "gob/surface.h"
#include "gob/video.h"
#include "gob/dataio.h"
#include "gob/rxyfile.h"
#include "gob/cmpfile.h"

namespace Gob {

CMPFile::CMPFile(GobEngine *vm, const Common::String &baseName,
                 uint16 width, uint16 height, uint8 bpp) :
	_vm(vm), _width(width), _height(height), _bpp(bpp), _maxWidth(0), _maxHeight(0),
	_surface(0), _coordinates(0) {

	if (baseName.empty())
		return;

	const Common::String rxyFile = Util::setExtension(baseName, ".RXY");
	const Common::String cmpFile = Util::setExtension(baseName, ".CMP");

	if (!_vm->_dataIO->hasFile(cmpFile))
		return;

	loadRXY(rxyFile);
	createSurface();

	loadCMP(cmpFile);
}

CMPFile::CMPFile(GobEngine *vm, const Common::String &cmpFile, const Common::String &rxyFile,
                 uint16 width, uint16 height, uint8 bpp) :
	_vm(vm), _width(width), _height(height), _bpp(bpp), _maxWidth(0), _maxHeight(0),
	_surface(0), _coordinates(0) {

	if (cmpFile.empty() || !_vm->_dataIO->hasFile(cmpFile))
		return;

	loadRXY(rxyFile);
	createSurface();

	loadCMP(cmpFile);
}

CMPFile::CMPFile(GobEngine *vm, Common::SeekableReadStream &cmp, Common::SeekableReadStream &rxy,
                 uint16 width, uint16 height, uint8 bpp) :
	_vm(vm), _width(width), _height(height), _bpp(bpp), _maxWidth(0), _maxHeight(0),
	_surface(0), _coordinates(0) {

	loadRXY(rxy);
	createSurface();

	loadCMP(cmp);
}

CMPFile::CMPFile(GobEngine *vm, Common::SeekableReadStream &cmp,
                 uint16 width, uint16 height, uint8 bpp) :
	_vm(vm), _width(width), _height(height), _bpp(bpp), _maxWidth(0), _maxHeight(0),
	_surface(0), _coordinates(0) {

	createRXY();
	createSurface();

	loadCMP(cmp);
}

CMPFile::~CMPFile() {
	delete _surface;
	delete _coordinates;
}

bool CMPFile::empty() const {
	return (_surface == 0) || (_coordinates == 0);
}

uint16 CMPFile::getSpriteCount() const {
	if (empty())
		return 0;

	return _coordinates->size();
}

void CMPFile::loadCMP(const Common::String &cmp) {
	Common::SeekableReadStream *dataCMP = _vm->_dataIO->getFile(cmp);
	if (!dataCMP)
		return;

	loadCMP(*dataCMP);

	delete dataCMP;
}

void CMPFile::loadRXY(const Common::String &rxy) {
	Common::SeekableReadStream *dataRXY = 0;
	if (!rxy.empty())
		dataRXY = _vm->_dataIO->getFile(rxy);

	if (dataRXY)
		loadRXY(*dataRXY);
	else
		createRXY();

	_height = _coordinates->getHeight();

	delete dataRXY;
}

void CMPFile::loadCMP(Common::SeekableReadStream &cmp) {
	uint32 size = cmp.size();
	byte  *data = new byte[size];

	if (cmp.read(data, size) != size)
		return;

	_vm->_video->drawPackedSprite(data, _surface->getWidth(), _surface->getHeight(), 0, 0, 0, *_surface);

	delete[] data;
}

void CMPFile::loadRXY(Common::SeekableReadStream &rxy) {
	bool bigEndian = (_vm->getEndiannessMethod() == kEndiannessMethodBE) ||
	                 ((_vm->getEndiannessMethod() == kEndiannessMethodSystem) &&
	                  (_vm->getEndianness() == kEndiannessBE));

	Common::SeekableSubReadStreamEndian sub(&rxy, 0, rxy.size(), bigEndian, DisposeAfterUse::NO);

	_coordinates = new RXYFile(sub);

	for (uint i = 0; i < _coordinates->size(); i++) {
		const RXYFile::Coordinates &c = (*_coordinates)[i];

		if (c.left == 0xFFFF)
			continue;

		const uint16 width  = c.right  - c.left + 1;
		const uint16 height = c.bottom - c.top  + 1;

		_maxWidth  = MAX(_maxWidth , width);
		_maxHeight = MAX(_maxHeight, height);
	}
}

void CMPFile::createRXY() {
	_coordinates = new RXYFile(_width, _height);

	_maxWidth  = _width;
	_maxHeight = _height;
}

void CMPFile::createSurface() {
	if (_width == 0)
		_width = 320;
	if (_height == 0)
		_height = 200;

	_surface = new Surface(_width, _height, _bpp);
}

bool CMPFile::getCoordinates(uint16 sprite, uint16 &left, uint16 &top, uint16 &right, uint16 &bottom) const {
	if (empty() || (sprite >= _coordinates->size()))
		return false;

	left   = (*_coordinates)[sprite].left;
	top    = (*_coordinates)[sprite].top;
	right  = (*_coordinates)[sprite].right;
	bottom = (*_coordinates)[sprite].bottom;

	return left != 0xFFFF;
}

uint16 CMPFile::getWidth(uint16 sprite) const {
	if (empty() || (sprite >= _coordinates->size()))
		return 0;

	return (*_coordinates)[sprite].right  - (*_coordinates)[sprite].left + 1;
}

uint16 CMPFile::getHeight(uint16 sprite) const {
	if (empty() || (sprite >= _coordinates->size()))
		return 0;

	return (*_coordinates)[sprite].bottom - (*_coordinates)[sprite].top  + 1;
}

void CMPFile::getMaxSize(uint16 &width, uint16 &height) const {
	width  = _maxWidth;
	height = _maxHeight;
}

void CMPFile::draw(Surface &dest, uint16 sprite, uint16 x, uint16 y, int32 transp) const {
	if (empty())
		return;

	if (sprite >= _coordinates->size())
		return;

	const RXYFile::Coordinates &coords = (*_coordinates)[sprite];

	draw(dest, coords.left, coords.top, coords.right, coords.bottom, x, y, transp);
}

void CMPFile::draw(Surface &dest, uint16 left, uint16 top, uint16 right, uint16 bottom,
                   uint16 x, uint16 y, int32 transp) const {

	if (!_surface)
		return;

	if (left == 0xFFFF)
		return;

	dest.blit(*_surface, left, top, right, bottom, x, y, transp);
}

uint16 CMPFile::addSprite(uint16 left, uint16 top, uint16 right, uint16 bottom) {
	if (empty())
		return 0;

	const uint16 height = bottom - top  + 1;
	const uint16 width  = right  - left + 1;

	_maxWidth  = MAX(_maxWidth , width);
	_maxHeight = MAX(_maxHeight, height);

	return _coordinates->add(left, top, right, bottom);
}

void CMPFile::recolor(uint8 from, uint8 to) {
	if (_surface)
		_surface->recolor(from, to);
}

} // End of namespace Gob
