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

#ifndef AGI_VIEW_H
#define AGI_VIEW_H

namespace Agi {

struct ViewCel {
	uint8 height;
	uint8 width;
	uint8 transparency;
	uint8 mirrorLoop;
	uint8 mirror;
	uint8 *data;
};

struct ViewLoop {
	int numCels;
	struct ViewCel *cel;
};

/**
 * AGI view resource structure.
 */
struct AgiView {
	int numLoops;
	struct ViewLoop *loop;
	bool agi256_2;
	char *descr;
	uint8 *rdata;
};

enum MotionType {
	kMotionNormal = 0,
	kMotionWander = 1,
	kMotionFollowEgo = 2,
	kMotionMoveObj = 3
};

enum CycleType {
	kCycleNormal = 0,
	kCycleEndOfLoop = 1,
	kCycleRevLoop = 2,
	kCycleReverse = 3
 };

enum ViewFlags {
	fDrawn 			= (1 << 0),
	fIgnoreBlocks 	= (1 << 1),
	fFixedPriority	= (1 << 2),
	fIgnoreHorizon	= (1 << 3),
	fUpdate			= (1 << 4),
	fCycling		= (1 << 5),
	fAnimated		= (1 << 6),
	fMotion			= (1 << 7),
	fOnWater		= (1 << 8),
	fIgnoreObjects	= (1 << 9),
	fUpdatePos		= (1 << 10),
	fOnLand			= (1 << 11),
	fDontupdate		= (1 << 12),
	fFixLoop		= (1 << 13),
	fDidntMove		= (1 << 14),
	fAdjEgoXY		= (1 << 15)
};

/**
 * AGI view table entry
 */
struct VtEntry {
	uint8 stepTime;
	uint8 stepTimeCount;
	uint8 entry;
	int16 xPos;
	int16 yPos;
	uint8 currentView;
	bool viewReplaced;
	struct AgiView *viewData;
	uint8 currentLoop;
	uint8 numLoops;
	struct ViewLoop *loopData;
	uint8 currentCel;
	uint8 numCels;
	struct ViewCel *celData;
	struct ViewCel *celData2;
	int16 xPos2;
	int16 yPos2;
	void *s;
	int16 xSize;
	int16 ySize;
	uint8 stepSize;
	uint8 cycleTime;
	uint8 cycleTimeCount;
	uint8 direction;
	MotionType motion;
	CycleType cycle;
	uint8 priority;
	uint16 flags;
	uint8 parm1;
	uint8 parm2;
	uint8 parm3;
	uint8 parm4;
}; // struct vt_entry

} // End of namespace Agi

#endif /* AGI_VIEW_H */
