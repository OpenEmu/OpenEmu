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

#include "scumm/scumm.h"
#include "scumm/actor.h"
#include "scumm/boxes.h"
#include "scumm/resource.h"
#include "scumm/scumm_v0.h"
#include "scumm/scumm_v6.h"
#include "scumm/util.h"

#include "common/util.h"

namespace Scumm {

#include "common/pack-start.h"	// START STRUCT PACKING

struct Box {				/* Internal walkbox file format */
	union {
		struct {
			byte x1;
			byte x2;
			byte y1;
			byte y2;
			byte mask;
		} v0;

		struct {
			byte uy;
			byte ly;
			byte ulx;
			byte urx;
			byte llx;
			byte lrx;
			byte mask;
			byte flags;
		} v2;

		struct {
			int16 ulx, uly;
			int16 urx, ury;
			int16 lrx, lry;
			int16 llx, lly;
			byte mask;
			byte flags;
			uint16 scale;
		} old;

		struct {
			int32 ulx, uly;
			int32 urx, ury;
			int32 lrx, lry;
			int32 llx, lly;
			uint32 mask;
			uint32 flags;
			uint32 scaleSlot;
			uint32 scale;
			uint32 unk2;
			uint32 unk3;
		} v8;
	};
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

#define BOX_MATRIX_SIZE 2000
#define BOX_DEBUG 0


static void getGates(const BoxCoords &box1, const BoxCoords &box2, Common::Point gateA[2], Common::Point gateB[2]);

static bool compareSlope(const Common::Point &p1, const Common::Point &p2, const Common::Point &p3) {
	return (p2.y - p1.y) * (p3.x - p1.x) <= (p3.y - p1.y) * (p2.x - p1.x);
}

/**
 * Find the point on a line segment which is closest to a given point.
 *
 * @param lineStart	the start of the line segment
 * @param lineEnd	the end of the line segment
 * @param p		the point which we want to 'project' on the line segment
 * @return the point on the line segment closest to the given point
 */
static Common::Point closestPtOnLine(const Common::Point &lineStart, const Common::Point &lineEnd, const Common::Point &p) {
	Common::Point result;

	const int lxdiff = lineEnd.x - lineStart.x;
	const int lydiff = lineEnd.y - lineStart.y;

	if (lineEnd.x == lineStart.x) {	// Vertical line?
		result.x = lineStart.x;
		result.y = p.y;
	} else if (lineEnd.y == lineStart.y) {	// Horizontal line?
		result.x = p.x;
		result.y = lineStart.y;
	} else {
		const int dist = lxdiff * lxdiff + lydiff * lydiff;
		int a, b, c;
		if (ABS(lxdiff) > ABS(lydiff)) {
			a = lineStart.x * lydiff / lxdiff;
			b = p.x * lxdiff / lydiff;

			c = (a + b - lineStart.y + p.y) * lydiff * lxdiff / dist;

			result.x = c;
			result.y = c * lydiff / lxdiff - a + lineStart.y;
		} else {
			a = lineStart.y * lxdiff / lydiff;
			b = p.y * lydiff / lxdiff;

			c = (a + b - lineStart.x + p.x) * lydiff * lxdiff / dist;

			result.x = c * lxdiff / lydiff - a + lineStart.x;
			result.y = c;
		}
	}

	if (ABS(lydiff) < ABS(lxdiff)) {
		if (lxdiff > 0) {
			if (result.x < lineStart.x)
				result = lineStart;
			else if (result.x > lineEnd.x)
				result = lineEnd;
		} else {
			if (result.x > lineStart.x)
				result = lineStart;
			else if (result.x < lineEnd.x)
				result = lineEnd;
		}
	} else {
		if (lydiff > 0) {
			if (result.y < lineStart.y)
				result = lineStart;
			else if (result.y > lineEnd.y)
				result = lineEnd;
		} else {
			if (result.y > lineStart.y)
				result = lineStart;
			else if (result.y < lineEnd.y)
				result = lineEnd;
		}
	}

	return result;
}

byte ScummEngine::getMaskFromBox(int box) {
	// WORKAROUND for bug #740244 and #755863. This appears to have been a
	// long standing bug in the original engine?
	if (_game.version <= 3 && box == 255)
		return 1;

	Box *ptr = getBoxBaseAddr(box);
	if (!ptr)
		return 0;

	// WORKAROUND for bug #847827: This is a bug in the data files, as it also
	// occurs with the original engine. We work around it here anyway.
	if (_game.id == GID_INDY4 && _currentRoom == 225 && _roomResource == 94 && box == 8)
		return 0;

	if (_game.version == 8)
		return (byte) FROM_LE_32(ptr->v8.mask);
	else if (_game.version == 0)
		return ptr->v0.mask;
	else if (_game.version <= 2)
		return ptr->v2.mask;
	else
		return ptr->old.mask;
}

void ScummEngine::setBoxFlags(int box, int val) {
	debug(2, "setBoxFlags(%d, 0x%02x)", box, val);

	/* SCUMM7+ stuff */
	if (val & 0xC000) {
		assert(box >= 0 && box < 65);
		_extraBoxFlags[box] = val;
	} else {
		Box *ptr = getBoxBaseAddr(box);
		if (!ptr)
			return;
		if (_game.version == 8)
			ptr->v8.flags = TO_LE_32(val);
		else if (_game.version <= 2)
			ptr->v2.flags = val;
		else
			ptr->old.flags = val;
	}
}

byte ScummEngine::getBoxFlags(int box) {
	Box *ptr = getBoxBaseAddr(box);
	if (!ptr)
		return 0;
	if (_game.version == 8)
		return (byte) FROM_LE_32(ptr->v8.flags);
	else if (_game.version == 0)
		return 0;
	else if (_game.version <= 2)
		return ptr->v2.flags;
	else
		return ptr->old.flags;
}

void ScummEngine::setBoxScale(int box, int scale) {
	Box *ptr = getBoxBaseAddr(box);
	assert(ptr);
	if (_game.version == 8)
		ptr->v8.scale = TO_LE_32(scale);
	else if (_game.version <= 2)
		error("This should not ever be called");
	else
		ptr->old.scale = TO_LE_16(scale);
}

void ScummEngine::setBoxScaleSlot(int box, int slot) {
	Box *ptr = getBoxBaseAddr(box);
	assert(ptr);
	ptr->v8.scaleSlot = TO_LE_32(slot);
}

int ScummEngine::getScale(int box, int x, int y) {
	if (_game.version <= 3)
		return 255;

	Box *ptr = getBoxBaseAddr(box);
	if (!ptr)
		return 255;

	int slot = 0;
	int scale;

	if (_game.version == 8) {
		// COMI has a separate field for the scale slot...
		slot = FROM_LE_32(ptr->v8.scaleSlot);
		scale = FROM_LE_32(ptr->v8.scale);
	} else {
		scale = READ_LE_UINT16(&ptr->old.scale);
		if (scale & 0x8000)
			slot = (scale & 0x7FFF) + 1;
	}

	// Was a scale slot specified? If so, we compute the effective scale
	// from it, ignoring the box scale.
	if (slot)
		scale = getScaleFromSlot(slot, x, y);

	return scale;
}


int ScummEngine::getScaleFromSlot(int slot, int x, int y) {
	assert(1 <= slot && slot <= ARRAYSIZE(_scaleSlots));
  int scale;
	int scaleX = 0, scaleY = 0;
	ScaleSlot &s = _scaleSlots[slot-1];

	if (s.y1 == s.y2 && s.x1 == s.x2)
		error("Invalid scale slot %d", slot);

	if (s.y1 != s.y2) {
		if (y < 0)
			y = 0;

		scaleY = (s.scale2 - s.scale1) * (y - s.y1) / (s.y2 - s.y1) + s.scale1;
	}
	if (s.x1 == s.x2) {
		scale = scaleY;
	} else {
		scaleX = (s.scale2 - s.scale1) * (x - s.x1) / (s.x2 - s.x1) + s.scale1;

		if (s.y1 == s.y2) {
			scale = scaleX;
		} else {
			scale = (scaleX + scaleY) / 2;
		}
	}

	// Clip the scale to range 1-255
	if (scale < 1)
		scale = 1;
	else if (scale > 255)
		scale = 255;

	return scale;
}

int ScummEngine::getBoxScale(int box) {
	if (_game.version <= 3)
		return 255;
	Box *ptr = getBoxBaseAddr(box);
	if (!ptr)
		return 255;
	if (_game.version == 8)
		return FROM_LE_32(ptr->v8.scale);
	else
		return READ_LE_UINT16(&ptr->old.scale);
}

/**
 * Convert a rtScaleTable resource to a corresponding scale slot entry.
 *
 * At some point, we discovered that the old scale items (stored in rtScaleTable
 * resources) are in fact the same as (or rather, a predecessor of) the
 * scale slots used in COMI. While not being precomputed (and thus slightly
 * slower), scale slots are more flexible, and most importantly, can cope with
 * rooms higher than 200 pixels. That's an essential feature for DIG and FT
 * and in fact the lack of it caused various bugs in the past.
 *
 * Hence, we decided to switch all games to use the more powerful scale slots.
 * To accomodate old savegames, we attempt here to convert rtScaleTable
 * resources to scale slots.
 */
void ScummEngine::convertScaleTableToScaleSlot(int slot) {
	assert(1 <= slot && slot <= ARRAYSIZE(_scaleSlots));

	byte *resptr = getResourceAddress(rtScaleTable, slot);
	int lowerIdx, upperIdx;
	float m, oldM;

	// Do nothing if the given scale table doesn't exist
	if (resptr == 0)
		return;

	if (resptr[0] == resptr[199]) {
		// The scale is constant. This usually means we encountered one of the
		// "broken" cases. We set pseudo scale item values which lead to a
		// constant scale of 255.
		setScaleSlot(slot, 0, 0, 255, 0, 199, 255);
		return;
	}

	/*
	 * Essentially, what we are doing here is some kind of "line fitting"
	 * algorithm. The data in the scale table represents a linear graph. What
	 * we want to find is the slope and (vertical) offset of this line. Things
	 * are complicated by the fact that the line is cut of vertically at 1 and
	 * 255. We have to be careful in handling this and some border cases.
	 *
	 * Some typical graphs look like these:
	 *       ---         _--     ---
	 *      /         _--           \
	 *  ___/       _--               \___
	 *
	 * The method used here is to compute the slope of secants fixed at the
	 * left and right end. For most cases this detects the cut-over points
	 * quite accurately.
	 */

	// Search for the bend on the left side
	m = (resptr[199] - resptr[0]) / 199.0f;
	for (lowerIdx = 0; lowerIdx < 199 && (resptr[lowerIdx] == 1 || resptr[lowerIdx] == 255); lowerIdx++) {
		oldM = m;
		m = (resptr[199] - resptr[lowerIdx+1]) / (float)(199 - (lowerIdx+1));
		if (m > 0) {
			if (m <= oldM)
				break;
		} else {
			if (m >= oldM)
				break;
		}
	}

	// Search for the bend on the right side
	m = (resptr[199] - resptr[0]) / 199.0f;
	for (upperIdx = 199; upperIdx > 1 && (resptr[upperIdx] == 1 || resptr[upperIdx] == 255); upperIdx--) {
		oldM = m;
		m = (resptr[upperIdx-1] - resptr[0]) / (float)(upperIdx-1);
		if (m > 0) {
			if (m <= oldM)
				break;
		} else {
			if (m >= oldM)
				break;
		}
	}

	// If lowerIdx and upperIdx are equal, we assume that there
	// was no bend at all, and go for the maximum range.
	if (lowerIdx == upperIdx) {
		lowerIdx = 0;
		upperIdx = 199;
	}

	// The values of y1 and y2, as well as the scale, can now easily be computed
	setScaleSlot(slot, 0, lowerIdx, resptr[lowerIdx], 0, upperIdx, resptr[upperIdx]);

#if 0
	// Compute the variance, for debugging. It shouldn't exceed 1
	ScaleSlot &s = _scaleSlots[slot-1];
	int y;
	int sum = 0;
	int scale;
	float variance;
	for (y = 0; y < 200; y++) {
		scale = (s.scale2 - s.scale1) * (y - s.y1) / (s.y2 - s.y1) + s.scale1;
		if (scale < 1)
			scale = 1;
		else if (scale > 255)
			scale = 255;

		sum += (resptr[y] - scale) * (resptr[y] - scale);
	}
	variance = sum / (200.0 - 1.0);
	if (variance > 1)
		debug(1, "scale item %d, variance %f exceeds 1 (room %d)", slot, variance, _currentRoom);
#endif
}

void ScummEngine::setScaleSlot(int slot, int x1, int y1, int scale1, int x2, int y2, int scale2) {
	assert(1 <= slot && slot <= ARRAYSIZE(_scaleSlots));
	ScaleSlot &s = _scaleSlots[slot-1];
	s.x2 = x2;
	s.y2 = y2;
	s.scale2 = scale2;
	s.x1 = x1;
	s.y1 = y1;
	s.scale1 = scale1;
}

byte ScummEngine::getNumBoxes() {
	byte *ptr = getResourceAddress(rtMatrix, 2);
	if (!ptr)
		return 0;
	if (_game.version == 8)
		return (byte)READ_LE_UINT32(ptr);
	else if (_game.version >= 5)
		return (byte)READ_LE_UINT16(ptr);
	else
		return ptr[0];
}

Box *ScummEngine::getBoxBaseAddr(int box) {
	byte *ptr = getResourceAddress(rtMatrix, 2);
	if (!ptr || box == 255)
		return NULL;

	// WORKAROUND: The NES version of Maniac Mansion attempts to set flags for boxes 2-4
	// when there are only three boxes (0-2) when walking out to the garage.
	if ((_game.id == GID_MANIAC) && (_game.platform == Common::kPlatformNES) && (box >= ptr[0]))
		return NULL;

	// WORKAROUND: In "pass to adventure", the loom demo, when bobbin enters
	// the tent to the elders, box = 2, but ptr[0] = 2 -> errors out.
	// Also happens in Indy3EGA (see bug #770351) and ZakEGA (see bug #771803).
	//
	// This *might* mean that we have a bug in our box implementation
	// OTOH, the original engine, unlike ScummVM, performed no bound
	// checking at all. All the problems so far have been cases where
	// the value was exactly one more than what we consider the maximum.
	// So it seems to be most likely that all of these are script errors.
	//
	// As a workaround, we simply use the last box if the last+1 box is requested.
	// Note that this may cause different behavior than the original game
	// engine exhibited! To faithfully reproduce the behavior of the original
	// engine, we would have to know the data coming *after* the walkbox table.
	if (_game.version <= 4 && ptr[0] == box)
		box--;

	assertRange(0, box, ptr[0] - 1, "box");
	if (_game.version == 0)
		return (Box *)(ptr + box * SIZEOF_BOX_V0 + 1);
	else if (_game.version <= 2)
		return (Box *)(ptr + box * SIZEOF_BOX_V2 + 1);
	else if (_game.version == 3)
		return (Box *)(ptr + box * SIZEOF_BOX_V3 + 1);
	else if (_game.features & GF_SMALL_HEADER)
		return (Box *)(ptr + box * SIZEOF_BOX + 1);
	else if (_game.version == 8)
		return (Box *)(ptr + box * SIZEOF_BOX_V8 + 4);
	else
		return (Box *)(ptr + box * SIZEOF_BOX + 2);
}

int ScummEngine_v6::getSpecialBox(int x, int y) {
	int i;
	int numOfBoxes;
	byte flag;

	numOfBoxes = getNumBoxes() - 1;

	for (i = numOfBoxes; i >= 0; i--) {
		flag = getBoxFlags(i);

		if (!(flag & kBoxInvisible) && (flag & kBoxPlayerOnly))
			return (-1);

		if (checkXYInBoxBounds(i, x, y))
			return (i);
	}

	return (-1);
}

bool ScummEngine::checkXYInBoxBounds(int boxnum, int x, int y) {
	// Since this method is called by many other methods that take params
	// from e.g. script opcodes, but do not validate the boxnum, we
	// make a check here to filter out invalid boxes.
	// See also bug #1599113.
	if (boxnum < 0 || boxnum == Actor::kInvalidBox)
		return false;

	BoxCoords box = getBoxCoordinates(boxnum);
	const Common::Point p(x, y);

	// Quick check: If the x (resp. y) coordinate of the point is
	// strictly smaller (bigger) than the x (y) coordinates of all
	// corners of the quadrangle, then it certainly is *not* contained
	// inside the quadrangle.
	if (x < box.ul.x && x < box.ur.x && x < box.lr.x && x < box.ll.x)
		return false;

	if (x > box.ul.x && x > box.ur.x && x > box.lr.x && x > box.ll.x)
		return false;

	if (y < box.ul.y && y < box.ur.y && y < box.lr.y && y < box.ll.y)
		return false;

	if (y > box.ul.y && y > box.ur.y && y > box.lr.y && y > box.ll.y)
		return false;

	// Corner case: If the box is a simple line segment, we consider the
	// point to be contained "in" (or rather, lying on) the line if it
	// is very close to its projection to the line segment.
	if ((box.ul == box.ur && box.lr == box.ll) ||
		(box.ul == box.ll && box.ur == box.lr)) {

		Common::Point tmp;
		tmp = closestPtOnLine(box.ul, box.lr, p);
		if (p.sqrDist(tmp) <= 4)
			return true;
	}

	// Finally, fall back to the classic algorithm to compute containment
	// in a convex polygon: For each (oriented) side of the polygon
	// (quadrangle in this case), compute whether p is "left" or "right"
	// from it.

	if (!compareSlope(box.ul, box.ur, p))
		return false;

	if (!compareSlope(box.ur, box.lr, p))
		return false;

	if (!compareSlope(box.lr, box.ll, p))
		return false;

	if (!compareSlope(box.ll, box.ul, p))
		return false;

	return true;
}

BoxCoords ScummEngine::getBoxCoordinates(int boxnum) {
	BoxCoords tmp, *box = &tmp;
	Box *bp = getBoxBaseAddr(boxnum);
	assert(bp);

	if (_game.version == 8) {
		box->ul.x = (short)FROM_LE_32(bp->v8.ulx);
		box->ul.y = (short)FROM_LE_32(bp->v8.uly);
		box->ur.x = (short)FROM_LE_32(bp->v8.urx);
		box->ur.y = (short)FROM_LE_32(bp->v8.ury);

		box->ll.x = (short)FROM_LE_32(bp->v8.llx);
		box->ll.y = (short)FROM_LE_32(bp->v8.lly);
		box->lr.x = (short)FROM_LE_32(bp->v8.lrx);
		box->lr.y = (short)FROM_LE_32(bp->v8.lry);

		// WORKAROUND (see patch #684732): Some walkboxes in CMI appear
		// to have been flipped, in the sense that for instance the
		// lower boundary is above the upper one. We work around this
		// by simply flipping them back.

		if (box->ul.y > box->ll.y && box->ur.y > box->lr.y) {
			SWAP(box->ul, box->ll);
			SWAP(box->ur, box->lr);
		}

		if (box->ul.x > box->ur.x && box->ll.x > box->lr.x) {
			SWAP(box->ul, box->ur);
			SWAP(box->ll, box->lr);
		}
	} else if (_game.version == 0) {
		box->ul.x = bp->v0.x1;
		box->ul.y = bp->v0.y1;
		box->ur.x = bp->v0.x2;
		box->ur.y = bp->v0.y1;

		box->ll.x = bp->v0.x1;
		box->ll.y = bp->v0.y2;
		box->lr.x = bp->v0.x2;
		box->lr.y = bp->v0.y2;

		if ((bp->v0.mask & 0x88) == 0x88) {
			// walkbox for (right/left) corner
			if (bp->v0.mask & 0x04)
				box->ur = box->ul;
			else
				box->ul = box->ur;
		}
	} else if (_game.version <= 2) {
		box->ul.x = bp->v2.ulx;
		box->ul.y = bp->v2.uy;
		box->ur.x = bp->v2.urx;
		box->ur.y = bp->v2.uy;

		box->ll.x = bp->v2.llx;
		box->ll.y = bp->v2.ly;
		box->lr.x = bp->v2.lrx;
		box->lr.y = bp->v2.ly;
	} else {
		box->ul.x = (int16)READ_LE_UINT16(&bp->old.ulx);
		box->ul.y = (int16)READ_LE_UINT16(&bp->old.uly);
		box->ur.x = (int16)READ_LE_UINT16(&bp->old.urx);
		box->ur.y = (int16)READ_LE_UINT16(&bp->old.ury);

		box->ll.x = (int16)READ_LE_UINT16(&bp->old.llx);
		box->ll.y = (int16)READ_LE_UINT16(&bp->old.lly);
		box->lr.x = (int16)READ_LE_UINT16(&bp->old.lrx);
		box->lr.y = (int16)READ_LE_UINT16(&bp->old.lry);
	}
	return *box;
}

int getClosestPtOnBox(const BoxCoords &box, int x, int y, int16& outX, int16& outY) {
	const Common::Point p(x, y);
	Common::Point tmp;
	uint dist;
	uint bestdist = 0xFFFFFF;

	tmp = closestPtOnLine(box.ul, box.ur, p);
	dist = p.sqrDist(tmp);
	if (dist < bestdist) {
		bestdist = dist;
		outX = tmp.x;
		outY = tmp.y;
	}

	tmp = closestPtOnLine(box.ur, box.lr, p);
	dist = p.sqrDist(tmp);
	if (dist < bestdist) {
		bestdist = dist;
		outX = tmp.x;
		outY = tmp.y;
	}

	tmp = closestPtOnLine(box.lr, box.ll, p);
	dist = p.sqrDist(tmp);
	if (dist < bestdist) {
		bestdist = dist;
		outX = tmp.x;
		outY = tmp.y;
	}

	tmp = closestPtOnLine(box.ll, box.ul, p);
	dist = p.sqrDist(tmp);
	if (dist < bestdist) {
		bestdist = dist;
		outX = tmp.x;
		outY = tmp.y;
	}

	return bestdist;
}

byte *ScummEngine::getBoxMatrixBaseAddr() {
	byte *ptr = getResourceAddress(rtMatrix, 1);
	assert(ptr);
	if (*ptr == 0xFF)
		ptr++;
	return ptr;
}

/**
 * Compute if there is a way that connects box 'from' with box 'to'.
 * Returns the number of a box adjacent to 'from' that is the next on the
 * way to 'to' (this can be 'to' itself or a third box).
 * If there is no connection -1 is return.
 */
int ScummEngine::getNextBox(byte from, byte to) {
	const byte *boxm;
	byte i;
	const int numOfBoxes = getNumBoxes();
	int dest = -1;

	if (from == to)
		return to;

	if (to == Actor::kInvalidBox)
		return -1;

	if (from == Actor::kInvalidBox)
		return to;

	assert(from < numOfBoxes);
	assert(to < numOfBoxes);

	boxm = getBoxMatrixBaseAddr();

	if (_game.version == 0) {
		// calculate shortest paths
		byte *itineraryMatrix = (byte *)malloc(numOfBoxes * numOfBoxes);
		calcItineraryMatrix(itineraryMatrix, numOfBoxes);

		dest = to;
		do {
			dest = itineraryMatrix[numOfBoxes * from + dest];
		} while (dest != Actor::kInvalidBox && !areBoxesNeighbors(from, dest));

		if (dest == Actor::kInvalidBox)
			dest = -1;

		free(itineraryMatrix);
		return dest;
	} else if (_game.version <= 2) {
		// The v2 box matrix is a real matrix with numOfBoxes rows and columns.
		// The first numOfBoxes bytes contain indices to the start of the corresponding
		// row (although that seems unnecessary to me - the value is easily computable.
		boxm += numOfBoxes + boxm[from];
		return (int8)boxm[to];
	}

	// WORKAROUND #1: It seems that in some cases, the box matrix is corrupt
	// (more precisely, is too short) in the datafiles already. In
	// particular this seems to be the case in room 46 of Indy3 EGA (see
	// also bug #770690). This didn't cause problems in the original
	// engine, because there, the memory layout is different. After the
	// walkbox would follow the rest of the room file, thus the program
	// always behaved the same (and by chance, correct). Not so for us,
	// since random data may follow after the resource in ScummVM.
	//
	// As a workaround, we add a check for the end of the box matrix
	// resource, and abort the search once we reach the end.
	const byte *end = boxm + getResourceSize(rtMatrix, 1);

	// WORKAROUND #2: In addition to the above, we have to add this special
	// case to fix the scene in Indy3 where Indy meets Hitler in Berlin.
	// See bug #770690 and also bug #774783.
	if ((_game.id == GID_INDY3) && _roomResource == 46 && from == 1 && to == 0)
		return 0;

	// Skip up to the matrix data for box 'from'
	for (i = 0; i < from && boxm < end; i++) {
		while (boxm < end && *boxm != 0xFF)
			boxm += 3;
		boxm++;
	}

	// Now search for the entry for box 'to'
	while (boxm < end && boxm[0] != 0xFF) {
		if (boxm[0] <= to && to <= boxm[1])
			dest = (int8)boxm[2];
		boxm += 3;
	}

	if (boxm >= end)
		debug(0, "The box matrix apparently is truncated (room %d)", _roomResource);

	return dest;
}

/*
 * Computes the next point actor a has to walk towards in a straight
 * line in order to get from box1 to box3 via box2.
 */
bool Actor::findPathTowards(byte box1nr, byte box2nr, byte box3nr, Common::Point &foundPath) {
	assert(_vm->_game.version >= 3);
	BoxCoords box1 = _vm->getBoxCoordinates(box1nr);
	BoxCoords box2 = _vm->getBoxCoordinates(box2nr);
	Common::Point tmp;
	int i, j;
	int flag;
	int q, pos;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			if (box1.ul.x == box1.ur.x && box1.ul.x == box2.ul.x && box1.ul.x == box2.ur.x) {
				flag = 0;
				if (box1.ul.y > box1.ur.y) {
					SWAP(box1.ul.y, box1.ur.y);
					flag |= 1;
				}

				if (box2.ul.y > box2.ur.y) {
					SWAP(box2.ul.y, box2.ur.y);
					flag |= 2;
				}

				if (box1.ul.y > box2.ur.y || box2.ul.y > box1.ur.y ||
						((box1.ur.y == box2.ul.y || box2.ur.y == box1.ul.y) &&
						box1.ul.y != box1.ur.y && box2.ul.y != box2.ur.y)) {
					if (flag & 1)
						SWAP(box1.ul.y, box1.ur.y);
					if (flag & 2)
						SWAP(box2.ul.y, box2.ur.y);
				} else {
					pos = _pos.y;
					if (box2nr == box3nr) {
						int diffX = _walkdata.dest.x - _pos.x;
						int diffY = _walkdata.dest.y - _pos.y;
						int boxDiffX = box1.ul.x - _pos.x;

						if (diffX != 0) {
							int t;

							diffY *= boxDiffX;
							t = diffY / diffX;
							if (t == 0 && (diffY <= 0 || diffX <= 0)
									&& (diffY >= 0 || diffX >= 0))
								t = -1;
							pos = _pos.y + t;
						}
					}

					q = pos;
					if (q < box2.ul.y)
						q = box2.ul.y;
					if (q > box2.ur.y)
						q = box2.ur.y;
					if (q < box1.ul.y)
						q = box1.ul.y;
					if (q > box1.ur.y)
						q = box1.ur.y;
					if (q == pos && box2nr == box3nr)
						return true;
					foundPath.y = q;
					foundPath.x = box1.ul.x;
					return false;
				}
			}

			if (box1.ul.y == box1.ur.y && box1.ul.y == box2.ul.y && box1.ul.y == box2.ur.y) {
				flag = 0;
				if (box1.ul.x > box1.ur.x) {
					SWAP(box1.ul.x, box1.ur.x);
					flag |= 1;
				}

				if (box2.ul.x > box2.ur.x) {
					SWAP(box2.ul.x, box2.ur.x);
					flag |= 2;
				}

				if (box1.ul.x > box2.ur.x || box2.ul.x > box1.ur.x ||
						((box1.ur.x == box2.ul.x || box2.ur.x == box1.ul.x) &&
						box1.ul.x != box1.ur.x && box2.ul.x != box2.ur.x)) {
					if (flag & 1)
						SWAP(box1.ul.x, box1.ur.x);
					if (flag & 2)
						SWAP(box2.ul.x, box2.ur.x);
				} else {

					if (box2nr == box3nr) {
						int diffX = _walkdata.dest.x - _pos.x;
						int diffY = _walkdata.dest.y - _pos.y;
						int boxDiffY = box1.ul.y - _pos.y;

						pos = _pos.x;
						if (diffY != 0) {
							pos += diffX * boxDiffY / diffY;
						}
					} else {
						pos = _pos.x;
					}

					q = pos;
					if (q < box2.ul.x)
						q = box2.ul.x;
					if (q > box2.ur.x)
						q = box2.ur.x;
					if (q < box1.ul.x)
						q = box1.ul.x;
					if (q > box1.ur.x)
						q = box1.ur.x;
					if (q == pos && box2nr == box3nr)
						return true;
					foundPath.x = q;
					foundPath.y = box1.ul.y;
					return false;
				}
			}
			tmp = box1.ul;
			box1.ul = box1.ur;
			box1.ur = box1.lr;
			box1.lr = box1.ll;
			box1.ll = tmp;
		}
		tmp = box2.ul;
		box2.ul = box2.ur;
		box2.ur = box2.lr;
		box2.lr = box2.ll;
		box2.ll = tmp;
	}
	return false;
}

#if BOX_DEBUG
static void printMatrix(byte *boxm, int num) {
	int i;
	for (i = 0; i < num; i++) {
		debugN("%d: ", i);
		while (*boxm != 0xFF) {
			debug("%d, ", *boxm);
			boxm++;
		}
		boxm++;
		debug("\n");
	}
}

static void printMatrix2(byte *matrix, int num) {
	int i, j;
	debug("    ");
	for (i = 0; i < num; i++)
		debug("%2d ", i);
	debug("\n");
	for (i = 0; i < num; i++) {
		debug("%2d: ", i);
		for (j = 0; j < num; j++) {
			int val = matrix[i * num + j];
			if (val == Actor::kInvalidBox)
				debug(" ? ");
			else
				debug("%2d ", val);
		}
		debug("\n");
	}
}
#endif

/**
 * Computes shortest paths and stores them in the itinerary matrix.
 * Parameter "num" holds the number of rows (= number of columns).
 */
void ScummEngine::calcItineraryMatrix(byte *itineraryMatrix, int num) {
	int i, j, k;
	byte *adjacentMatrix;

	const uint8 boxSize = (_game.version == 0) ? num : 64;

	// Allocate the adjacent & itinerary matrices
	adjacentMatrix = (byte *)malloc(boxSize * boxSize);

	// Initialize the adjacent matrix: each box has distance 0 to itself,
	// and distance 1 to its direct neighbors. Initially, it has distance
	// 255 (= infinity) to all other boxes.
	for (i = 0; i < num; i++) {
		for (j = 0; j < num; j++) {
			if (i == j) {
				adjacentMatrix[i * boxSize + j] = 0;
				itineraryMatrix[i * boxSize + j] = j;
			} else if (areBoxesNeighbors(i, j)) {
				adjacentMatrix[i * boxSize + j] = 1;
				itineraryMatrix[i * boxSize + j] = j;
			} else {
				adjacentMatrix[i * boxSize + j] = 255;
				itineraryMatrix[i * boxSize + j] = Actor::kInvalidBox;
			}
		}
	}

	// Compute the shortest routes between boxes via Kleene's algorithm.
	// The original code used some kind of mangled Dijkstra's algorithm;
	// while that might in theory be slightly faster, it was
	// a) extremly obfuscated
	// b) incorrect: it didn't always find the shortest paths
	// c) not any faster in reality for our sparse & small adjacent matrices
	for (k = 0; k < num; k++) {
		for (i = 0; i < num; i++) {
			for (j = 0; j < num; j++) {
				if (i == j)
					continue;
				byte distIK = adjacentMatrix[boxSize * i + k];
				byte distKJ = adjacentMatrix[boxSize * k + j];
				if (adjacentMatrix[boxSize * i + j] > distIK + distKJ) {
					adjacentMatrix[boxSize * i + j] = distIK + distKJ;
					itineraryMatrix[boxSize * i + j] = itineraryMatrix[boxSize * i + k];
				}
			}
		}

	}

	free(adjacentMatrix);
}

void ScummEngine::createBoxMatrix() {
	int num, i, j;

	// The total number of boxes
	num = getNumBoxes();

	const uint8 boxSize = (_game.version == 0) ? num : 64;

	// calculate shortest paths
	byte *itineraryMatrix = (byte *)malloc(boxSize * boxSize);
	calcItineraryMatrix(itineraryMatrix, num);

	// "Compress" the distance matrix into the box matrix format used
	// by the engine. The format is like this:
	// For each box (from 0 to num) there is first a byte with value 0xFF,
	// followed by an arbitrary number of byte triples; the end is marked
	// again by the lead 0xFF for the next "row". The meaning of the
	// byte triples is as follows: the first two bytes define a range
	// of box numbers (e.g. 7-11), while the third byte defines an
	// itineray box. Assuming we are in the 5th "row" and encounter
	// the triplet 7,11,15: this means to get from box 5 to any of
	// the boxes 7,8,9,10,11 the shortest way is to go via box 15.
	// See also getNextBox.

	byte *matrixStart = _res->createResource(rtMatrix, 1, BOX_MATRIX_SIZE);
	const byte *matrixEnd = matrixStart + BOX_MATRIX_SIZE;

	#define addToMatrix(b)	do { *matrixStart++ = (b); assert(matrixStart < matrixEnd); } while (0)

	for (i = 0; i < num; i++) {
		addToMatrix(0xFF);
		for (j = 0; j < num; j++) {
			byte itinerary = itineraryMatrix[boxSize * i + j];
			if (itinerary != Actor::kInvalidBox) {
				addToMatrix(j);
				while (j < num - 1 && itinerary == itineraryMatrix[boxSize * i + (j + 1)])
					j++;
				addToMatrix(j);
				addToMatrix(itinerary);
			}
		}
	}
	addToMatrix(0xFF);


#if BOX_DEBUG
	debug("Itinerary matrix:\n");
	printMatrix2(itineraryMatrix, num);
	debug("compressed matrix:\n");
	printMatrix(getBoxMatrixBaseAddr(), num);
#endif

	free(itineraryMatrix);
}

/** Check if two boxes are neighbors. */
bool ScummEngine::areBoxesNeighbors(int box1nr, int box2nr) {
	Common::Point tmp;
	BoxCoords box;
	BoxCoords box2;

	if ((getBoxFlags(box1nr) & kBoxInvisible) || (getBoxFlags(box2nr) & kBoxInvisible))
		return false;

	assert(_game.version >= 3);
	box2 = getBoxCoordinates(box1nr);
	box = getBoxCoordinates(box2nr);

	// Roughly, the idea of this algorithm is to search for sies of the given
	// boxes that touch each other.
	// In order to keep te code simple, we only match the upper sides;
	// then, we "rotate" the box coordinates four times each, for a total
	// of 16 comparisions.
	for (int j = 0; j < 4; j++) {
		for (int k = 0; k < 4; k++) {
			// Are the "upper" sides of the boxes on a single vertical line
			// (i.e. all share one x value) ?
			if (box2.ur.x == box2.ul.x && box.ul.x == box2.ul.x && box.ur.x == box2.ul.x) {
				bool swappedBox2 = false, swappedBox1 = false;
				if (box2.ur.y < box2.ul.y) {
					swappedBox2 = true;
					SWAP(box2.ur.y, box2.ul.y);
				}
				if (box.ur.y < box.ul.y) {
					swappedBox1 = true;
					SWAP(box.ur.y, box.ul.y);
				}
				if (box.ur.y < box2.ul.y ||
						box.ul.y > box2.ur.y ||
						((box.ul.y == box2.ur.y ||
						 box.ur.y == box2.ul.y) && box2.ur.y != box2.ul.y && box.ul.y != box.ur.y)) {
				} else {
					return true;
				}

				// Swap back if necessary
				if (swappedBox2) {
					SWAP(box2.ur.y, box2.ul.y);
				}
				if (swappedBox1) {
					SWAP(box.ur.y, box.ul.y);
				}
			}

			// Are the "upper" sides of the boxes on a single horizontal line
			// (i.e. all share one y value) ?
			if (box2.ur.y == box2.ul.y && box.ul.y == box2.ul.y && box.ur.y == box2.ul.y) {
				bool swappedBox2 = false, swappedBox1 = false;
				if (box2.ur.x < box2.ul.x) {
					swappedBox2 = true;
					SWAP(box2.ur.x, box2.ul.x);
				}
				if (box.ur.x < box.ul.x) {
					swappedBox1 = true;
					SWAP(box.ur.x, box.ul.x);
				}
				if (box.ur.x < box2.ul.x ||
						box.ul.x > box2.ur.x ||
						((box.ul.x == box2.ur.x ||
						 box.ur.x == box2.ul.x) && box2.ur.x != box2.ul.x && box.ul.x != box.ur.x)) {

				} else {
					return true;
				}

				// Swap back if necessary
				if (swappedBox2) {
					SWAP(box2.ur.x, box2.ul.x);
				}
				if (swappedBox1) {
					SWAP(box.ur.x, box.ul.x);
				}
			}

			// "Rotate" the box coordinates
			tmp = box2.ul;
			box2.ul = box2.ur;
			box2.ur = box2.lr;
			box2.lr = box2.ll;
			box2.ll = tmp;
		}

		// "Rotate" the box coordinates
		tmp = box.ul;
		box.ul = box.ur;
		box.ur = box.lr;
		box.lr = box.ll;
		box.ll = tmp;
	}

	return false;
}

bool ScummEngine_v0::areBoxesNeighbors(int box1nr, int box2nr) {
	int i;
	const int numOfBoxes = getNumBoxes();
	const byte *boxm;

	assert(box1nr < numOfBoxes);
	assert(box2nr < numOfBoxes);

	boxm = getBoxMatrixBaseAddr();
	// TODO: what are the first bytes for (mostly 0)?
	boxm += 4;

	// For each box, the matrix contains an arbitrary number
	// of box indices that are linked with the box (neighbors).
	// Each list is separated by 0xFF (|).
	// E.g. "1 | 0 3 | 3 | 1 2" means:
	//   0 -> 1, 1 -> 0/3, 2 -> 3, 3 -> 1/2

	// Skip up to the matrix data for box 'box1nr'
	for (i = 0; i < box1nr; i++) {
		while (*boxm != 0xFF)
			boxm++;
		boxm++;
	}

	// Now search for the entry for box 'box2nr'
	while (boxm[0] != 0xFF) {
		if (boxm[0] == box2nr)
			return true;
		boxm++;
	}

	return false;
}

void Actor_v3::findPathTowardsOld(byte box1, byte box2, byte finalBox, Common::Point &p2, Common::Point &p3) {
	Common::Point gateA[2];
	Common::Point gateB[2];

	getGates(_vm->getBoxCoordinates(box1), _vm->getBoxCoordinates(box2), gateA, gateB);

	p2.x = 32000;
	p3.x = 32000;

	// next box (box2) = final box?
	if (box2 == finalBox) {
		// In Indy3, the masks (= z-level) have to match, too -- needed for the
		// 'maze' in the zeppelin (see bug #1032964).
		if (_vm->_game.id != GID_INDY3 || _vm->getMaskFromBox(box1) == _vm->getMaskFromBox(box2)) {
			// Is the actor (x,y) between both gates?
			if (compareSlope(_pos, _walkdata.dest, gateA[0]) !=
					compareSlope(_pos, _walkdata.dest, gateB[0]) &&
					compareSlope(_pos, _walkdata.dest, gateA[1]) !=
					compareSlope(_pos, _walkdata.dest, gateB[1])) {
				return;
			}
		}
	}

	p3 = closestPtOnLine(gateA[1], gateB[1], _pos);

	if (compareSlope(_pos, p3, gateA[0]) == compareSlope(_pos, p3, gateB[0])) {
		p2 = closestPtOnLine(gateA[0], gateB[0], _pos);
	}
}

/**
 * Compute the "gate" between two boxes. The gate is a pair of two lines which
 * both start on box 'box1' and end on 'box2'. For both lines, one of its
 * end points is the corner point of one of the two boxes. The other end point
 * is the point on the other box closest to the first end point.
 * This way the lines form the boundary of a 'corridor' between the two boxes,
 * through which the actor has to walk to get from box1 to box2.
 */
void getGates(const BoxCoords &box1, const BoxCoords &box2, Common::Point gateA[2], Common::Point gateB[2]) {
	int i, j;
	int dist[8];
	int minDist[3];
	int closest[3];
	int box[3];
	Common::Point closestPoint[8];
	Common::Point boxCorner[8];
	int line1, line2;

	// For all corner coordinates of the first box, compute the point closest
	// to them on the second box (and also compute the distance of these points).
	boxCorner[0] = box1.ul;
	boxCorner[1] = box1.ur;
	boxCorner[2] = box1.lr;
	boxCorner[3] = box1.ll;
	for (i = 0; i < 4; i++) {
		dist[i] = getClosestPtOnBox(box2, boxCorner[i].x, boxCorner[i].y, closestPoint[i].x, closestPoint[i].y);
	}

	// Now do the same but with the roles of the first and second box swapped.
	boxCorner[4] = box2.ul;
	boxCorner[5] = box2.ur;
	boxCorner[6] = box2.lr;
	boxCorner[7] = box2.ll;
	for (i = 4; i < 8; i++) {
		dist[i] = getClosestPtOnBox(box1, boxCorner[i].x, boxCorner[i].y, closestPoint[i].x, closestPoint[i].y);
	}

	// Find the three closest "close" points between the two boxes.
	for (j = 0; j < 3; j++) {
		minDist[j] = 0xFFFF;
		for (i = 0; i < 8; i++) {
			if (dist[i] < minDist[j]) {
				minDist[j] = dist[i];
				closest[j] = i;
			}
		}
		dist[closest[j]] = 0xFFFF;
		minDist[j] = (int)sqrt((double)minDist[j]);
		box[j] = (closest[j] > 3);	// Is the point on the first or on the second box?
	}


	// Finally, compute the actual "gate".

	if (box[0] == box[1] && ABS(minDist[0] - minDist[1]) < 4) {
		line1 = closest[0];
		line2 = closest[1];

	} else if (box[0] == box[1] && minDist[0] == minDist[1]) {	// parallel
		line1 = closest[0];
		line2 = closest[1];
	} else if (box[0] == box[2] && minDist[0] == minDist[2]) {	// parallel
		line1 = closest[0];
		line2 = closest[2];
	} else if (box[1] == box[2] && minDist[1] == minDist[2]) {	// parallel
		line1 = closest[1];
		line2 = closest[2];

	} else if (box[0] == box[2] && ABS(minDist[0] - minDist[2]) < 4) {
		line1 = closest[0];
		line2 = closest[2];
	} else if (ABS(minDist[0] - minDist[2]) < 4) {
		line1 = closest[1];
		line2 = closest[2];
	} else if (ABS(minDist[0] - minDist[1]) < 4) {
		line1 = closest[0];
		line2 = closest[1];
	} else {
		line1 = closest[0];
		line2 = closest[0];
	}

	// Set the gate
	if (line1 < 4) {
		gateA[0] = boxCorner[line1];
		gateA[1] = closestPoint[line1];
	} else {
		gateA[1] = boxCorner[line1];
		gateA[0] = closestPoint[line1];
	}

	if (line2 < 4) {
		gateB[0] = boxCorner[line2];
		gateB[1] = closestPoint[line2];
	} else {
		gateB[1] = boxCorner[line2];
		gateB[0] = closestPoint[line2];
	}
}

} // End of namespace Scumm
