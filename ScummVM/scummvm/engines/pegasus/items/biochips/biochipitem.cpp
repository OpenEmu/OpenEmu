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


#include "common/stream.h"

#include "pegasus/pegasus.h"
#include "pegasus/ai/ai_area.h"
#include "pegasus/items/biochips/biochipitem.h"

namespace Pegasus {

BiochipItem::BiochipItem(const ItemID id, const NeighborhoodID neighborhood, const RoomID room, const DirectionConstant direction) :
		Item(id, neighborhood, room, direction) {

	PegasusEngine *vm = (PegasusEngine *)g_engine;

	Common::SeekableReadStream *biochipInfo = vm->_resFork->getResource(MKTAG('B', 'i', 'o', 'I'), kItemBaseResID + id);
	if (biochipInfo) {
		_biochipInfoPanelTime = biochipInfo->readUint32BE();
		delete biochipInfo;
	} else {
		_biochipInfoPanelTime = 0;
	}

	Common::SeekableReadStream *rightInfo = vm->_resFork->getResource(MKTAG('R', 'g', 'h', 't'), kItemBaseResID + id);
	if (!rightInfo)
		error("Could not find right info for biochip %d", id);

	_rightAreaInfo = readItemState(rightInfo);
	delete rightInfo;

	setItemState(kNormalItem);
}

BiochipItem::~BiochipItem() {
	delete[] _rightAreaInfo.entries;
}

ItemType BiochipItem::getItemType() {
	return kBiochipItemType;
}

TimeValue BiochipItem::getRightAreaTime() const {
	if (!_rightAreaInfo.entries)
		return 0xffffffff;

	TimeValue time;
	ItemState state;

	findItemStateEntryByState(_rightAreaInfo, _itemState, time);
	if (time == 0xffffffff)
		getItemStateEntry(_rightAreaInfo, 0, state, time);

	return time;
}

//	Must affect images in right area.
void BiochipItem::select() {
	Item::select();

	if (g_AIArea)
		g_AIArea->setAIAreaToTime(kBiochipSignature, kRightAreaSignature, getRightAreaTime());
}

void BiochipItem::deselect() {
	Item::deselect();

	if (g_AIArea)
		g_AIArea->setAIAreaToTime(kBiochipSignature, kRightAreaSignature, 0xffffffff);
}

} // End of namespace Pegasus
