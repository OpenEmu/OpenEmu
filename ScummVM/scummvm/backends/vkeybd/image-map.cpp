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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#ifdef ENABLE_VKEYBD

#include "common/textconsole.h"

#include "backends/vkeybd/image-map.h"
#include "backends/vkeybd/polygon.h"

namespace Common {

ImageMap::~ImageMap() {
	removeAllAreas();
}

Polygon *ImageMap::createArea(const String &id) {
	if (_areas.contains(id)) {
		warning("Image map already contains an area with target of '%s'", id.c_str());
		return 0;
	}
	Polygon *p = new Polygon();
	_areas[id] = p;
	return p;
}

void ImageMap::removeArea(const String &id) {
	if (!_areas.contains(id))
		return;
	delete _areas[id];
	_areas.erase(id);
}

void ImageMap::removeAllAreas() {
	for (AreaMap::iterator it = _areas.begin(); it != _areas.end(); ++it) {
		delete it->_value;
	}
	_areas.clear();
}

String ImageMap::findMapArea(int16 x, int16 y) {
	for (AreaMap::iterator it = _areas.begin(); it != _areas.end(); ++it) {
		if (it->_value->contains(x, y))
			return it->_key;
	}

	return String();
}


} // End of namespace Common

#endif // #ifdef ENABLE_VKEYBD
