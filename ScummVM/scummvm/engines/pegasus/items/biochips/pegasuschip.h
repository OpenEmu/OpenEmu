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

#ifndef PEGASUS_ITEMS_BIOCHIPS_PEGASUSCHIP_H
#define PEGASUS_ITEMS_BIOCHIPS_PEGASUSCHIP_H

#include "pegasus/hotspot.h"
#include "pegasus/items/biochips/biochipitem.h"

namespace Pegasus {

class PegasusChip : public BiochipItem {
public:
	PegasusChip(const ItemID, const NeighborhoodID, const RoomID, const DirectionConstant);
	virtual ~PegasusChip();

	void select();

	void setUpPegasusChip();

	// Called to set up the Pegasus chip when the Pegasus chip is the current chip but does not
	// own the center area.
	void setUpPegasusChipRude();
	void activatePegasusHotspots();
	void clickInPegasusHotspot();

protected:
	Hotspot _recallSpot;
};

} // End of namespace Pegasus

#endif
