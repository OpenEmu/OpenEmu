/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#include "pegasus/gamestate.h"
#include "pegasus/ai/ai_area.h"
#include "pegasus/items/biochips/mapchip.h"
#include "pegasus/neighborhood/neighborhood.h"

namespace Pegasus {

MapChip *g_map = 0;

MapChip::MapChip(const ItemID id, const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant direction) :
		BiochipItem(id, neighborhood, room, direction) {
	g_map = this;
	setItemState(kMapUnavailable);
}

MapChip::~MapChip() {
	g_map = 0;
}

void MapChip::writeToStream(Common::WriteStream *stream) {
	return _image.writeToStream(stream);
}

void MapChip::readFromStream(Common::ReadStream *stream) {
	return _image.readFromStream(stream);
}

void MapChip::select() {
	BiochipItem::select();
	moveToMapLocation(GameState.getCurrentNeighborhood(), GameState.getCurrentRoom(), GameState.getCurrentDirection());
	_image.show();
}

void MapChip::takeSharedArea() {
	_image.show();
}

void MapChip::giveUpSharedArea() {
	_image.hide();
}

void MapChip::deselect() {
	BiochipItem::deselect();
	_image.unloadImage();
}

void MapChip::moveToMapLocation(const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant dir) {
	AirQuality airQuality;

	if (g_neighborhood)
		airQuality = g_neighborhood->getAirQuality(room);
	else
		airQuality = kAirQualityGood;

	switch (neighborhood) {
	case kMarsID:
		if (airQuality == kAirQualityVacuum) {
			if (room >= kMars35 && room <= kMars39) {
				setItemState(kMapEngaged);
				if (isSelected() && g_AIArea && g_AIArea->getMiddleAreaOwner() == kBiochipSignature)
					_image.loadGearRoomIfNecessary();
			} else {
				setItemState(kMapEngaged);
				if (isSelected() && g_AIArea && g_AIArea->getMiddleAreaOwner() == kBiochipSignature)
					_image.loadMazeIfNecessary();
			}

			_image.moveToMapLocation(neighborhood, room, dir);
		} else {
			_image.unloadImage();
			setItemState(kMapUnavailable);
		}
		break;
	default:
		_image.unloadImage();
		setItemState(kMapUnavailable);
		break;
	}
}

} // End of namespace Pegasus
