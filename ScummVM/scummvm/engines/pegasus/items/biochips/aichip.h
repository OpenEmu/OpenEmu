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

#ifndef PEGASUS_ITEMS_BIOCHIPS_AICHIP_H
#define PEGASUS_ITEMS_BIOCHIPS_AICHIP_H

#include "pegasus/hotspot.h"
#include "pegasus/items/biochips/biochipitem.h"

namespace Pegasus {

class AIChip : public BiochipItem {
public:
	AIChip(const ItemID, const NeighborhoodID, const RoomID, const DirectionConstant);
	virtual ~AIChip();

	void select();

	void setUpAIChip();

	// Called to set up the AI chip when the AI chip is the current chip but does not
	// own the center area.
	void setUpAIChipRude();
	void activateAIHotspots();
	void clickInAIHotspot(HotSpotID);

	void takeSharedArea();

	void showBriefingClicked();
	void showEnvScanClicked();
	void clearClicked();

protected:
	Hotspot _briefingSpot;
	Hotspot _scanSpot;
	Hotspot _hint1Spot;
	Hotspot _hint2Spot;
	Hotspot _hint3Spot;
	Hotspot _solveSpot;
	bool _playingMovie;
};

extern AIChip *g_AIChip;

} // End of namespace Pegasus

#endif
