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

#ifndef SKY_AUTOROUTE_H
#define SKY_AUTOROUTE_H


#include "common/scummsys.h"

namespace Sky {

struct Compact;
class Grid;
class SkyCompact;

class AutoRoute {
public:
	AutoRoute(Grid *pGrid, SkyCompact *compact);
	~AutoRoute();
	uint16 autoRoute(Compact *cpt);
private:
	uint16 checkBlock(uint16 *blockPos);
	void clipCoordX(uint16 x, uint8 &blkX, int16 &initX);
	void clipCoordY(uint16 y, uint8 &blkY, int16 &initY);
	void initWalkGrid(uint8 screen, uint8 width);
	bool calcWalkGrid(uint8 startX, uint8 startY, uint8 destX, uint8 destY);
	uint16 *makeRouteData(uint8 startX, uint8 startY, uint8 destX, uint8 destY);
	uint16 *checkInitMove(uint16 *data, int16 initStaX);
	Grid *_grid;
	SkyCompact *_skyCompact;
	uint16 *_routeGrid;
	uint16 *_routeBuf;
	static const int16 _routeDirections[4];
	static const uint16 _logicCommands[4];
};

} // End of namespace Sky

#endif // AUTOROUTE_H
