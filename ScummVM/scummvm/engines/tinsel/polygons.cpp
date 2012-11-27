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
 */

#include "tinsel/actors.h"
#include "tinsel/font.h"
#include "tinsel/handle.h"
#include "tinsel/pcode.h"
#include "tinsel/pid.h"
#include "tinsel/polygons.h"
#include "tinsel/rince.h"
#include "tinsel/sched.h"
#include "common/serializer.h"
#include "tinsel/tinsel.h"
#include "tinsel/token.h"

#include "common/textconsole.h"
#include "common/util.h"

namespace Tinsel {


//----------------- LOCAL DEFINES --------------------

/** different types of polygon */
enum POLY_TYPE {
	POLY_PATH, POLY_NPATH, POLY_BLOCK, POLY_REFER, POLY_EFFECT,
	POLY_EXIT, POLY_TAG
};

// Note 7/10/94, with adjacency reduction ANKHMAP max is 3, UNSEEN max is 4
// so reduced this back to 6 (from 12) for now.
#define MAXADJ	6	// Max number of known adjacent paths

struct POLYGON {

	PTYPE	polyType;	// Polygon type

	int	subtype;	// refer type in REFER polygons
				// NODE/NORMAL in PATH polygons

	int	pIndex;		// Index into compiled polygon data

	/*
	 * Data duplicated from compiled polygon data
	 */
	short	cx[4];		// Corners (clockwise direction)
	short	cy[4];
	int	polyID;

	/* For TAG polygons only */
	int tagFlags;
	SCNHANDLE hOverrideTag;

	/* For TAG and EXIT (and EFFECT in future?) polygons only   */
	TSTATE	tagState;
	PSTATE	pointState;

	/* For Path polygons only  */
	bool	tried;

	/*
	 * Internal derived data for speed and conveniance
	 * set up by FiddlyBit()
	 */
	short	ptop;		//
	short	pbottom;	// Enclosing external rectangle
	short	pleft;		//
	short	pright;		//

	short	ltop[4];	//
	short	lbottom[4];	// Rectangles enclosing each side
	short	lleft[4];	//
	short	lright[4];	//

	int	a[4];		// y1-y2       }
	int	b[4];		// x2-x1       } See IsInPolygon()
	long	c[4];		// y1x2 - x1y2 }

	/*
	 * Internal derived data for speed and conveniance
	 * set up by PseudoCenter()
	 */
	int	pcenterx;	// Pseudo-center
	int	pcentery;	//

	/**
	 * List of adjacent polygons. For Path polygons only.
	 * set up by SetPathAdjacencies()
	 */
	POLYGON *adjpaths[MAXADJ];

};
typedef POLYGON *PPOLYGON;

#define MAXONROUTE 40

#include "common/pack-start.h"	// START STRUCT PACKING

/** lineinfo struct - one per (node-1) in a node path */
struct LINEINFO {

	int32	a;
	int32	b;
	int32	c;

	int32	a2;             ///< a squared
	int32	b2;             ///< b squared
	int32	a2pb2;          ///< a squared + b squared
	int32	ra2pb2;         ///< root(a squared + b squared)

	int32	ab;
	int32	ac;
	int32	bc;
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

/**
 * POLY structure class. This is implemented as a class, because the structure
 * of POLY's changed between TINSEL v1 and v2.
 *
 * FIXME: Right now, we always read *all* data in a polygon, even if only a single
 * field is needed. This is rather inefficient.
 */
class Poly {
private:
	const byte * const  _pStart;
	const byte *_pData;
	int _recordSize;
	void nextPoly();

public:
	Poly(const byte *pSrc);
	Poly(const byte *pSrc, int startIndex);
	void operator++();
	void setIndex(int index);


	POLY_TYPE getType() const { return (POLY_TYPE)FROM_LE_32(type); }
	int getNodecount() const { return (int)FROM_LE_32(nodecount); }
	int getNodeX(int i) const { return (int)FROM_LE_32(nlistx[i]); }
	int getNodeY(int i) const { return (int)FROM_LE_32(nlisty[i]); }

	// get Inter-node line structure
	const LINEINFO *getLineinfo(int i) const { return ((const LINEINFO *)(_pStart + (int)FROM_LE_32(plinelist))) + i; }

protected:
	POLY_TYPE type;		///< type of polygon
public:
	int32 x[4], y[4];	// Polygon definition
	uint32 xoff, yoff;	// DW2 - polygon offset

	int32 tagx, tagy;	// } For tagged polygons
	SCNHANDLE hTagtext;	// } i.e. EXIT, TAG, EFFECT

	int32 nodex, nodey;	// EXIT, TAG, REFER
	SCNHANDLE hFilm;	///< film reel handle for EXIT, TAG

	int32 reftype;		///< Type of REFER

	int32 id;			// } EXIT and TAG

	int32 scale1, scale2;	// }
	int32 level1, level2;	// DW2 fields
	int32 bright1, bright2;	// DW2 fields
	int32 reel;			// } PATH and NPATH
	int32 zFactor;		// }

protected:
	int32 nodecount;		///<The number of nodes in this polygon
	int32 pnodelistx, pnodelisty;	///<offset in chunk to this array if present
	int32 plinelist;

	const int32 *nlistx;
	const int32 *nlisty;

public:
	SCNHANDLE hScript;	///< handle of code segment for polygon events
};

Poly::Poly(const byte *pSrc) : _pStart(pSrc) {
	_pData = pSrc;
	nextPoly();
	_recordSize = _pData - pSrc;
}

Poly::Poly(const byte *pSrc, int startIndex) : _pStart(pSrc)  {
	_pData = pSrc;
	nextPoly();
	_recordSize = _pData - pSrc;
	setIndex(startIndex);
}

void Poly::operator++() {
	nextPoly();
}

void Poly::setIndex(int index) {
	_pData = _pStart + index * _recordSize;
	nextPoly();
}

static uint32 nextLong(const byte *&p) {
	uint32 result = READ_UINT32(p);
	p += 4;
	return result;
}

void Poly::nextPoly() {
	// Note: For now we perform no endian conversion of the data. We could change that
	// at some point, and remove all endian conversions from the code that uses POLY's
	const byte *pRecord = _pData;

	int typeVal = nextLong(_pData);
	if ((FROM_LE_32(typeVal) == 5) && TinselV2)
		typeVal = TO_LE_32(6);
	type = (POLY_TYPE)typeVal;

	for (int i = 0; i < 4; ++i)
		x[i] = nextLong(_pData);
	for (int i = 0; i < 4; ++i)
		y[i] = nextLong(_pData);

	if (TinselV2) {
		xoff = nextLong(_pData);
		yoff = nextLong(_pData);
		id = nextLong(_pData);
		reftype = nextLong(_pData);
	}

	tagx = nextLong(_pData);
	tagy = nextLong(_pData);
	hTagtext = nextLong(_pData);
	nodex = nextLong(_pData);
	nodey = nextLong(_pData);
	hFilm = nextLong(_pData);

	if (!TinselV2) {
		reftype = nextLong(_pData);
		id = nextLong(_pData);
	}

	scale1 = nextLong(_pData);
	scale2 = nextLong(_pData);

	if (TinselV2) {
		level1 = nextLong(_pData);
		level2 = nextLong(_pData);
		bright1 = nextLong(_pData);
		bright2 = nextLong(_pData);
	}

	reel = nextLong(_pData);
	zFactor = nextLong(_pData);
	nodecount = nextLong(_pData);
	pnodelistx = nextLong(_pData);
	pnodelisty = nextLong(_pData);
	plinelist = nextLong(_pData);

	nlistx = (const int32 *)(_pStart + (int)FROM_LE_32(pnodelistx));
	nlisty = (const int32 *)(_pStart + (int)FROM_LE_32(pnodelisty));

	if (TinselV0)
		// Skip to the last 4 bytes of the record for the hScript value
		_pData = pRecord + 0x62C;

	hScript = nextLong(_pData);
}

//----------------- LOCAL GLOBAL DATA --------------------

// FIXME: Avoid non-const global vars

static int MaxPolys = MAX_POLY;

static POLYGON *Polys[MAX_POLY+1];

static POLYGON *Polygons = 0;

static SCNHANDLE pHandle = 0;	// } Set at start of each scene
static int noofPolys = 0;		// }

static POLYGON extraBlock;	// Used for dynamic blocking

static int pathsOnRoute = 0;
static const POLYGON *RoutePaths[MAXONROUTE];

static POLYGON *RouteEnd = 0;

#ifdef DEBUG
int highestYet = 0;
#endif

// dead/alive, offsets
static POLY_VOLATILE volatileStuff[MAX_POLY];

//----------------- LOCAL MACROS --------------------

// The str parameter is no longer used
#define CHECK_HP_OR(mvar, str) assert((mvar >= 0 && mvar <= noofPolys) || mvar == MAX_POLY);
#define CHECK_HP(mvar, str)	assert(mvar >= 0 && mvar <= noofPolys);

//-------------------- METHODS ----------------------

static HPOLYGON PolygonIndex(const POLYGON *pp) {
	for (int i = 0; i <= MAX_POLY; ++i) {
		if (Polys[i] == pp)
			return i;
	}

	error("PolygonIndex(): polygon not found");
}

/**
 * Returns TRUE if the point is within the polygon supplied.
 *
 * Firstly, the point must be within the smallest imaginary rectangle
 * which encloses the polygon.
 *
 * Then, from each corner of the polygon, if the point is within an
 * imaginary rectangle enclosing the clockwise-going side from that
 * corner, the gradient of a line from the corner to the point must be
 * less than (or more negative than) the gradient of that side:
 *
 * If the corners' coordinates are designated (x1, y1) and (x2, y2), and
 * the point in question's (xt, yt), then:
 *     gradient (x1,y1)->(x2,y2) > gradient (x1,y1)->(xt,yt)
 *               (y1-y2)/(x2-x1) > (y1-yt)/(xt-x1)
 *               (y1-y2)*(xt-x1) > (y1-yt)*(x2-x1)
 *        xt(y1-y2) -x1y1 + x1y2 > -yt(x2-x1) + y1x2 - x1y1
 *         xt(y1-y2) + yt(x2-x1) > y1x2 - x1y2
 *
 * If the point passed one of the four 'side tests', and failed none,
 * then it must be within the polygon. If the point was not tested, it
 * may be within the internal rectangle not covered by the above tests.
 *
 * Most polygons contain an internal rectangle which does not fall into
 * any of the above side-related tests. Such a rectangle will always
 * have two polygon corners above it and two corners to the left of it.
 */
bool IsInPolygon(int xt, int yt, HPOLYGON hp) {
	const POLYGON *pp;
	int	i;
	bool BeenTested = false;
	int	pl = 0, pa = 0;

	CHECK_HP_OR(hp, "Out of range polygon handle (1)");
	pp = Polys[hp];
	assert(pp != NULL); // Testing whether in a NULL polygon

	// Shift cursor for relative polygons
	if (TinselV2) {
		xt -= volatileStuff[hp].xoff;
		yt -= volatileStuff[hp].yoff;
	}

	/* Is point within the external rectangle? */
	if (xt < pp->pleft || xt > pp->pright || yt < pp->ptop || yt > pp->pbottom)
		return false;

	// For each corner/side
	for (i = 0; i < 4; i++)	{
		// If within this side's 'testable' area
		// i.e. within the width of the line in y direction of end of line
		// or within the height of the line in x direction of end of line
		if ((xt >= pp->lleft[i] && xt <= pp->lright[i]  && ((yt > pp->cy[i]) == (pp->cy[(i+1)%4] > pp->cy[i])))
		 || (yt >= pp->ltop[i]  && yt <= pp->lbottom[i] && ((xt > pp->cx[i]) == (pp->cx[(i+1)%4] > pp->cx[i])))) {
			if (((long)xt*pp->a[i] + (long)yt*pp->b[i]) < pp->c[i])
				return false;
			else
				BeenTested = true;
		}
	}

	if (BeenTested) {
		// New dodgy code 29/12/94
		if (pp->polyType == BLOCK) {
			// For each corner/side
			for (i = 0; i < 4; i++) {
				// Pretend the corners of blocking polys are not in the poly.
				if (xt == pp->cx[i] && yt == pp->cy[i])
					return false;
			}
		}
		return true;
	} else {
		// Is point within the internal rectangle?
		for (i = 0; i < 4; i++) {
			if (pp->cx[i] < xt)
				pl++;
			if (pp->cy[i] < yt)
				pa++;
		}

		if (pa == 2 && pl == 2)
			return true;
		else
			return false;
	}
}

/**
 * Finds a polygon of the specified type containing the supplied point.
 */
HPOLYGON InPolygon(int xt, int yt, PTYPE type) {
	for (int j = 0; j <= MAX_POLY; j++)	{
		if (Polys[j] && Polys[j]->polyType == type) {
			if (IsInPolygon(xt, yt, j))
				return j;
		}
	}
	return NOPOLY;
}

/**
 * Given a blocking polygon, current co-ordinates of an actor, and the
 * co-ordinates of where the actor is heading, works out which corner of
 * the blocking polygon to head around.
 */

void BlockingCorner(HPOLYGON hp, int *x, int *y, int tarx, int tary) {
	const POLYGON *pp;
	int	i;
	int	xd, yd;		// distance per axis
	int	ThisD, SmallestD = 1000;
	int	D1, D2;
	int	NearestToHere = 1000, NearestToTarget;
	unsigned At = 10;	// Corner already at

	int	bcx[4], bcy[4];	// Bogus corners

	CHECK_HP_OR(hp, "Out of range polygon handle (2)");
	pp = Polys[hp];

	// Work out a point outside each corner
	for (i = 0; i < 4; i++)	{
		int	next, prev;

		// X-direction
		next = pp->cx[i] - pp->cx[(i+1)%4];
		prev = pp->cx[i] - pp->cx[(i+3)%4];
		if (next <= 0 && prev <= 0)
			bcx[i] = pp->cx[i] - 4;		// Both points to the right
		else if (next >= 0 && prev >= 0)
			bcx[i] = pp->cx[i] + 4;		// Both points to the left
		else
			bcx[i] = pp->cx[i];

		// Y-direction
		next = pp->cy[i] - pp->cy[(i+1)%4];
		prev = pp->cy[i] - pp->cy[(i+3)%4];
		if (next <= 0 && prev <= 0)
			bcy[i] = pp->cy[i] - 4;		// Both points below
		else if (next >= 0 && prev >= 0)
			bcy[i] = pp->cy[i] + 4;		// Both points above
		else
			bcy[i] = pp->cy[i];
	}

	// Find nearest corner to where we are,
	// but not the one we're stood at.

	for (i = 0; i < 4; i++) {		// For 4 corners
//		ThisD = ABS(*x - pp->cx[i]) + ABS(*y - pp->cy[i]);
		ThisD = ABS(*x - bcx[i]) + ABS(*y - bcy[i]);
		if (ThisD < SmallestD) {
			// Ignore this corner if it's not in a path
			if (InPolygon(pp->cx[i], pp->cy[i], PATH) == NOPOLY ||
			    InPolygon(bcx[i], bcy[i], PATH) == NOPOLY)
				continue;

			// Are we stood at this corner?
			if (ThisD > 4) {
				// No - it's the nearest we've found yet.
				NearestToHere = i;
				SmallestD = ThisD;
			} else {
				// Stood at/next to this corner
				At = i;
			}
		}
	}

	// If we're not already at a corner, go to the nearest corner

	if (At == 10) {
		// Not stood at a corner
//		assert(NearestToHere != 1000); // At blocking corner, not found near corner!
		// Better to give up than to assert fail!
		if (NearestToHere == 1000) {
			// Send it to where it is now
			// i.e. leave x and y alone
		} else {
			*x = bcx[NearestToHere];
			*y = bcy[NearestToHere];
		}
	} else {
		// Already at a corner. Go to an adjacent corner.
		// First, find out which adjacent corner is nearest the target.
			xd = ABS(tarx - pp->cx[(At + 1) % 4]);
			yd = ABS(tary - pp->cy[(At + 1) % 4]);
			D1 = xd + yd;
			xd = ABS(tarx - pp->cx[(At + 3) % 4]);
			yd = ABS(tary - pp->cy[(At + 3) % 4]);
			D2 = xd + yd;
			NearestToTarget = (D2 > D1) ? (At + 1) % 4 : (At + 3) % 4;
		if (NearestToTarget == NearestToHere) {
			*x = bcx[NearestToHere];
			*y = bcy[NearestToHere];
		} else {
			// Need to decide whether it's better to go to the nearest to
			// here and then on to the target, or to the nearest to the
			// target and on from there.
			xd = ABS(pp->cx[At] - pp->cx[NearestToHere]);
			D1 = xd;
			xd = ABS(pp->cx[NearestToHere] - tarx);
			D1 += xd;

			yd = ABS(pp->cy[At] - pp->cy[NearestToHere]);
			D1 += yd;
			yd = ABS(pp->cy[NearestToHere] - tary);
			D1 += yd;

			xd = ABS(pp->cx[At] - pp->cx[NearestToTarget]);
			D2 = xd;
			xd = ABS(pp->cx[NearestToTarget] - tarx);
			D2 += xd;

			yd = ABS(pp->cy[At] - pp->cy[NearestToTarget]);
			D2 += yd;
			yd = ABS(pp->cy[NearestToTarget] - tary);
			D2 += yd;

			if (D2 > D1) {
				*x = bcx[NearestToHere];
				*y = bcy[NearestToHere];
			} else {
				*x = bcx[NearestToTarget];
				*y = bcy[NearestToTarget];
			}
		}
	}
}


/**
 * Try do drop a perpendicular to each inter-node line from the point
 * and remember the shortest (if any).
 * Find which node is nearest to the point.
 * The shortest of these gives the best point in the node path.
*/
void FindBestPoint(HPOLYGON hp, int *x, int *y, int *pline) {
	const POLYGON *pp;

	int	dropD;		// length of perpendicular (i.e. distance of point from line)
	int	dropX, dropY;	// (X, Y) where dropped perpendicular intersects the line
	int	d1, d2;		// distance from perpendicular intersect to line's end nodes

	int	shortestD = 10000;	// Shortest distance found
	int	nearestL = -1;		// Nearest line
	int	nearestN;		// Nearest Node

	int	h = *x;		// For readability/conveniance
	int	k = *y;		//	- why aren't these #defines?

	CHECK_HP(hp, "Out of range polygon handle (3)");
	pp = Polys[hp];

	// Pointer to polygon data
	Poly ptp(LockMem(pHandle), pp->pIndex);	// This polygon

	// Look for fit of perpendicular to lines between nodes
	for (int i = 0; i < ptp.getNodecount() - 1; i++) {
		const LINEINFO *line = ptp.getLineinfo(i);

		const int32	a = (int)FROM_LE_32(line->a);
		const int32	b = (int)FROM_LE_32(line->b);
		const int32	c = (int)FROM_LE_32(line->c);

#if 1
		// TODO: If the comments of the LINEINFO struct are correct, then it contains mostly
		// duplicate data, probably in an effort to safe CPU cycles. Even on the slowest devices
		// we support, calculating a product of two ints is not an issue.
		// So we can just load & endian convert a,b,c, then replace stuff like
		//   (int)FROM_LE_32(line->ab)
		// by simply a*b, which makes it easier to understand what the code does, too.
		// Just in case there is some bugged data, I leave this code here for verifying it.
		// Let's leave it in for some time.
		//
		// One bad thing: We use sqrt to compute a square root. Might not be a good idea,
		// speed wise. Maybe we should take Vicent's fp_sqroot. But that's a problem for later.

		int32	a2 = (int)FROM_LE_32(line->a2);             ///< a squared
		int32	b2 = (int)FROM_LE_32(line->b2);             ///< b squared
		int32	a2pb2 = (int)FROM_LE_32(line->a2pb2);          ///< a squared + b squared
		int32	ra2pb2 = (int)FROM_LE_32(line->ra2pb2);         ///< root(a squared + b squared)

		int32	ab = (int)FROM_LE_32(line->ab);
		int32	ac = (int)FROM_LE_32(line->ac);
		int32	bc = (int)FROM_LE_32(line->bc);

		assert(a*a == a2);
		assert(b*b == b2);
		assert(a*b == ab);
		assert(a*c == ac);
		assert(b*c == bc);

		assert(a2pb2 == a*a + b*b);
		assert(ra2pb2 == (int)sqrt((float)a*a + (float)b*b));
#endif


		if (a == 0 && b == 0)
			continue;		// Line is just a point!

		// X position of dropped perpendicular intersection with line
		dropX = ((b*b * h) - (a*b * k) - a*c) / (a*a + b*b);

		// X distances from intersection to end nodes
		d1 = dropX - ptp.getNodeX(i);
		d2 = dropX - ptp.getNodeX(i+1);

		// if both -ve or both +ve, no fit
		if ((d1 < 0 && d2 < 0) || (d1 > 0 && d2 > 0))
			continue;
//#if 0
		// Y position of sidweays perpendicular intersection with line
		dropY = ((a*a * k) - (a*b * h) - b*c) / (a*a + b*b);

		// Y distances from intersection to end nodes
		d1 = dropY - ptp.getNodeY(i);
		d2 = dropY - ptp.getNodeY(i+1);

		// if both -ve or both +ve, no fit
		if ((d1 < 0 && d2 < 0) || (d1 > 0 && d2 > 0))
			continue;
//#endif
		dropD = ((a * h) + (b * k) + c) / (int)sqrt((float)a*a + (float)b*b);
		dropD = ABS(dropD);
		if (dropD < shortestD) {
			shortestD = dropD;
			nearestL = i;
		}
	}

	// Distance to nearest node
	nearestN = NearestNodeWithin(hp, h, k);
	dropD = ABS(h - ptp.getNodeX(nearestN)) + ABS(k - ptp.getNodeY(nearestN));

	// Go to a node or a point on a line
	if (dropD < shortestD) {
		// A node is nearest
		*x = ptp.getNodeX(nearestN);
		*y = ptp.getNodeY(nearestN);
		*pline = nearestN;
	} else {
		assert(nearestL != -1);

		// A point on a line is nearest
		const LINEINFO *line = ptp.getLineinfo(nearestL);
		const int32	a = (int)FROM_LE_32(line->a);
		const int32	b = (int)FROM_LE_32(line->b);
		const int32	c = (int)FROM_LE_32(line->c);
		dropX = ((b*b * h) - (a*b * k) - a*c) / (a*a + b*b);
		dropY = ((a*a * k) - (a*b * h) - b*c) / (a*a + b*b);
		*x = dropX;
		*y = dropY;
		*pline = nearestL;
	}

	assert(IsInPolygon(*x, *y, hp)); // Nearest point is not in polygon(!)
}

/**
 * Returns TRUE if two paths are asdjacent.
 */
bool IsAdjacentPath(HPOLYGON hPath1, HPOLYGON hPath2) {
	const POLYGON *pp1, *pp2;

	CHECK_HP(hPath1, "Out of range polygon handle (4)");
	CHECK_HP(hPath2, "Out of range polygon handle (500)");

	if (hPath1 == hPath2)
		return true;

	pp1 = Polys[hPath1];
	pp2 = Polys[hPath2];

	for (int j = 0; j < MAXADJ; j++)
		if (pp1->adjpaths[j] == pp2)
			return true;

	return false;
}

static const POLYGON *TryPath(POLYGON *last, POLYGON *whereto, POLYGON *current) {
	POLYGON *x;

	// For each path adjacent to this one
	for (int j = 0; j < MAXADJ; j++) {
		x = current->adjpaths[j];	// call the adj. path x
		if (x == whereto) {
			RoutePaths[pathsOnRoute++] = x;
			return x;		// Got there!
		}

		if (x == NULL)
			break;			// no more adj. paths to look at

		if (x->tried)
			continue;		// don't double back

		if (x == last)
			continue;		// don't double back

		x->tried = true;
		if (TryPath(current, whereto, x) != NULL) {
			RoutePaths[pathsOnRoute++] = x;
			assert(pathsOnRoute < MAXONROUTE);
			return x;		// Got there in this direction
		} else
			x->tried = false;
	}

	return NULL;
}


/**
 * Sort out the first path to head to for the imminent leg of a walk.
 */
static HPOLYGON PathOnTheWay(HPOLYGON from, HPOLYGON to) {
	// TODO: Fingolfin says: This code currently uses DFS (depth first search),
	// in the TryPath function, to compute a path between 'from' and 'to'.
	// However, a BFS (breadth first search) might yield more natural results,
	// at least in cases where there are multiple possible paths.
	// There is a small risk of regressions caused by such a change, though.
	//
	// Also, the overhead of computing a DFS again and again could be avoided
	// by computing a path matrix (like we do in the SCUMM engine).
	int	i;

	CHECK_HP(from, "Out of range polygon handle (501a)");
	CHECK_HP(to, "Out of range polygon handle (501b)");

	if (IsAdjacentPath(from, to))
		return to;

	for (i = 0; i < MAX_POLY; i++) {		// For each polygon..
		POLYGON *p = Polys[i];
		if (p && p->polyType == PATH)	//...if it's a path
			p->tried = false;
	}
	Polys[from]->tried = true;
	pathsOnRoute = 0;

	const POLYGON *p = TryPath(Polys[from], Polys[to], Polys[from]);

	if (TinselV2 && !p)
		return NOPOLY;

	assert(p != NULL); // Trying to find route between unconnected paths

	// Don't go a roundabout way to an adjacent path.
	for (i = 0; i < pathsOnRoute; i++) {
		CHECK_HP(PolygonIndex(RoutePaths[i]), "Out of range polygon handle (502)");
		if (IsAdjacentPath(from, PolygonIndex(RoutePaths[i])))
			return PolygonIndex(RoutePaths[i]);
	}
	return PolygonIndex(p);
}

/**
 * Indirect method of calling PathOnTheWay().
 * Used to be implemented using coroutines, to put the burden of
 * recursion onto the main stack. Since our "fake" coroutines use the
 * same stack for everything anyway, we can do without the coroutines.
 */
HPOLYGON GetPathOnTheWay(HPOLYGON hFrom, HPOLYGON hTo) {
	CHECK_HP(hFrom, "Out of range polygon handle (6)");
	CHECK_HP(hTo, "Out of range polygon handle (7)");

	// Reuse already computed result
	if (RouteEnd == Polys[hTo]) {
		for (int i = 0; i < pathsOnRoute; i++) {
			CHECK_HP(PolygonIndex(RoutePaths[i]), "Out of range polygon handle (503)");
			if (IsAdjacentPath(hFrom, PolygonIndex(RoutePaths[i]))) {
				return PolygonIndex(RoutePaths[i]);
			}
		}
	}

	RouteEnd = Polys[hTo];
	return PathOnTheWay(hFrom, hTo);
}


/**
 * Given a node path, work out which end node is nearest the given point.
 */
int NearestEndNode(HPOLYGON hPath, int x, int y) {
	const POLYGON *pp;

	int	d1, d2;

	CHECK_HP(hPath, "Out of range polygon handle (8)");
	pp = Polys[hPath];

	Poly ptp(LockMem(pHandle), pp->pIndex);	// This polygon

	const int nodecount = ptp.getNodecount() - 1;

	d1 = ABS(x - ptp.getNodeX(0)) + ABS(y - ptp.getNodeY(0));
	d2 = ABS(x - ptp.getNodeX(nodecount)) + ABS(y - ptp.getNodeY(nodecount));

	return (d2 > d1) ? 0 : nodecount;
}


/**
 * Given a start path and a destination path, find which pair of end
 * nodes is nearest together.
 * Return which node in the start path is part of the closest pair.
 */
int NearEndNode(HPOLYGON hSpath, HPOLYGON hDpath) {
	const POLYGON *pSpath, *pDpath;

	int	dist, NearDist;
	int	NearNode;

	CHECK_HP(hSpath, "Out of range polygon handle (9)");
	CHECK_HP(hDpath, "Out of range polygon handle (10)");
	pSpath = Polys[hSpath];
	pDpath = Polys[hDpath];

	uint8 *pps = LockMem(pHandle);		// All polygons
	Poly ps(pps, pSpath->pIndex);	// Start polygon
	Poly pd(pps, pDpath->pIndex);	// Dest polygon

	// 'top' nodes in each path
	const int ns = ps.getNodecount() - 1;
	const int nd = pd.getNodecount() - 1;

	// start[0] to dest[0]
	NearDist = ABS(ps.getNodeX(0) - pd.getNodeX(0)) + ABS(ps.getNodeY(0) - pd.getNodeY(0));
	NearNode = 0;

	// start[0] to dest[top]
	dist = ABS(ps.getNodeX(0) - pd.getNodeX(nd)) + ABS(ps.getNodeY(0) - pd.getNodeY(nd));
	if (dist < NearDist)
		NearDist = dist;

	// start[top] to dest[0]
	dist = ABS(ps.getNodeX(ns) - pd.getNodeX(0)) + ABS(ps.getNodeY(ns) - pd.getNodeY(0));
	if (dist < NearDist) {
		NearDist = dist;
		NearNode = ns;
	}

	// start[top] to dest[top]
	dist = ABS(ps.getNodeX(ns) - pd.getNodeX(nd)) + ABS(ps.getNodeY(ns) - pd.getNodeY(nd));
	if (dist < NearDist) {
		NearNode = ns;
	}

	return NearNode;
}

/**
 * Given a follow nodes path and a co-ordinate, finds which node in the
 * path is nearest to the co-ordinate.
 */
int NearestNodeWithin(HPOLYGON hNpath, int x, int y) {
	int	ThisDistance, SmallestDistance = 1000;
	int	NearestYet = 0;	// Number of nearest node

	CHECK_HP(hNpath, "Out of range polygon handle (11)");

	Poly ptp(LockMem(pHandle), Polys[hNpath]->pIndex);	// This polygon

	const int numNodes = ptp.getNodecount();	// Number of nodes in this follow nodes path

	for (int i = 0; i < numNodes; i++) {
		ThisDistance = ABS(x - ptp.getNodeX(i)) + ABS(y - ptp.getNodeY(i));

		if (ThisDistance < SmallestDistance) {
			NearestYet = i;
			SmallestDistance = ThisDistance;
		}
	}

	return NearestYet;
}

/**
 * Given a point and start and destination paths, find the nearest
 * corner (if any) of the start path which is within the destination
 * path. If there is no such corner, find the nearest corner of the
 * destination path which falls within the source path.
 */
void NearestCorner(int *x, int *y, HPOLYGON hStartPoly, HPOLYGON hDestPoly) {
	const POLYGON *psp, *pdp;
	int	j;
	int	ncorn = 0;			// nearest corner
	HPOLYGON hNpath = NOPOLY;	// path containing nearest corner
	int ThisD, SmallestD = 1000;

	CHECK_HP(hStartPoly, "Out of range polygon handle (12)");
	CHECK_HP(hDestPoly, "Out of range polygon handle (13)");

	psp = Polys[hStartPoly];
	pdp = Polys[hDestPoly];

	// Nearest corner of start path in destination path.

	for (j = 0; j < 4; j++)	{
		if (IsInPolygon(psp->cx[j], psp->cy[j], hDestPoly)) {
			ThisD = ABS(*x - psp->cx[j]) + ABS(*y - psp->cy[j]);
			if (ThisD < SmallestD) {
				hNpath = hStartPoly;
				ncorn = j;
				// Try to ignore it if virtually stood on it
				if (ThisD > 4)
					SmallestD = ThisD;
			}
		}
	}
	if (SmallestD == 1000) {
		// Nearest corner of destination path in start path.
		for (j = 0; j < 4; j++) {
			if (IsInPolygon(pdp->cx[j], pdp->cy[j], hStartPoly)) {
				ThisD = ABS(*x - pdp->cx[j]) + ABS(*y - pdp->cy[j]);
				if (ThisD < SmallestD) {
					hNpath = hDestPoly;
					ncorn = j;
					// Try to ignore it if virtually stood on it
					if (ThisD > 4)
						SmallestD = ThisD;
				}
			}
		}
	}

	if (hNpath != NOPOLY) {
		*x = Polys[hNpath]->cx[ncorn];
		*y = Polys[hNpath]->cy[ncorn];
	} else
		error("NearestCorner() failure");
}

bool IsPolyCorner(HPOLYGON hPath, int x, int y) {
	CHECK_HP(hPath, "Out of range polygon handle (37)");

	for (int i = 0; i < 4; i++)	{
		if (Polys[hPath]->cx[i] == x && Polys[hPath]->cy[i] == y)
			return true;
	}
	return false;
}

/**
 * Given a path polygon and a Y co-ordinate, return a scale value.
 */
int GetScale(HPOLYGON hPath, int y) {
	int	zones;		// Number of different scales
	int	zlen;		// Depth of each scale zone
	int	scale;
	int	top;

	// To try and fix some unknown potential bug
	if (hPath == NOPOLY)
		return SCALE_LARGE;

	CHECK_HP(hPath, "Out of range polygon handle (14)");

	Poly ptp(LockMem(pHandle), Polys[hPath]->pIndex);

	// Path is of a constant scale?
	if (FROM_LE_32(ptp.scale2) == 0)
		return FROM_LE_32(ptp.scale1);

	assert(FROM_LE_32(ptp.scale1) >= FROM_LE_32(ptp.scale2));

	zones = FROM_LE_32(ptp.scale1) - FROM_LE_32(ptp.scale2) + 1;
	zlen = (Polys[hPath]->pbottom - Polys[hPath]->ptop) / zones;

	scale = FROM_LE_32(ptp.scale1);
	top = Polys[hPath]->ptop;

	do {
		top += zlen;
		if (y < top)
			return scale;
	} while (--scale);

	return FROM_LE_32(ptp.scale2);
}

/**
 * Given a path polygon and a Y co-ordinate, return a brightness value.
 */

int GetBrightness(HPOLYGON hPath, int y) {
	int zones;		// Number of different brightnesses
	int zlen;		// Depth of each brightness zone
	int brightness;
	int top;

	// To try and fix some unknown potential bug
	if (hPath == NOPOLY)
		return 10;

	CHECK_HP(hPath, "Out of range polygon handle (38)");

	Poly ptp(LockMem(pHandle), Polys[hPath]->pIndex);

	// Path is of a constant brightness?
	if (FROM_LE_32(ptp.bright1) == FROM_LE_32(ptp.bright2))
		return FROM_LE_32(ptp.bright1);

	assert(FROM_LE_32(ptp.bright1) >= FROM_LE_32(ptp.bright2));

	zones = FROM_LE_32(ptp.bright1) - FROM_LE_32(ptp.bright2) + 1;
	zlen = (Polys[hPath]->pbottom - Polys[hPath]->ptop) / zones;

	brightness = FROM_LE_32(ptp.bright1);
	top = Polys[hPath]->ptop;

	do {
		top += zlen;
		if (y < top)
			return brightness;
	} while (--brightness);

	return FROM_LE_32(ptp.bright2);
}


/**
 * Give the co-ordinates of a node in a node path.
 */
void getNpathNode(HPOLYGON hNpath, int node, int *px, int *py) {
	CHECK_HP(hNpath, "Out of range polygon handle (15)");
	assert(Polys[hNpath] != NULL && Polys[hNpath]->polyType == PATH && Polys[hNpath]->subtype == NODE); // must be given a node path!

	Poly ptp(LockMem(pHandle), Polys[hNpath]->pIndex);	// This polygon

	// Might have just walked to the node from above.
	if (node == ptp.getNodecount())
		node -= 1;

	*px = ptp.getNodeX(node);
	*py = ptp.getNodeY(node);
}

/**
 * Get compiled tag text handle and tag co-ordinates of a tag polygon.
 */
void GetTagTag(HPOLYGON hp, SCNHANDLE *hTagText, int *tagx, int *tagy) {
	CHECK_HP(hp, "Out of range polygon handle (16)");

	Poly ptp(LockMem(pHandle), Polys[hp]->pIndex);

	*tagx = (int)FROM_LE_32(ptp.tagx) + (TinselV2 ? volatileStuff[hp].xoff : 0);
	*tagy = (int)FROM_LE_32(ptp.tagy) + (TinselV2 ? volatileStuff[hp].yoff : 0);
	*hTagText = FROM_LE_32(ptp.hTagtext);
}

/**
 * Get polygon's film reel handle.
 */
SCNHANDLE GetPolyFilm(HPOLYGON hp) {
	CHECK_HP(hp, "Out of range polygon handle (17)");

	Poly ptp(LockMem(pHandle), Polys[hp]->pIndex);

	return FROM_LE_32(ptp.hFilm);
}

/**
 * Get handle to polygon's glitter code.
 */
SCNHANDLE GetPolyScript(HPOLYGON hp) {
	CHECK_HP(hp, "Out of range polygon handle (19)");

	Poly ptp(LockMem(pHandle), Polys[hp]->pIndex);

	return FROM_LE_32(ptp.hScript);
}

REEL GetPolyReelType(HPOLYGON hp) {
	// To try and fix some unknown potential bug (toyshop entrance)
	if (hp == NOPOLY)
		return REEL_ALL;

	CHECK_HP(hp, "Out of range polygon handle (20)");

	Poly ptp(LockMem(pHandle), Polys[hp]->pIndex);

	return (REEL)FROM_LE_32(ptp.reel);
}

int32 GetPolyZfactor(HPOLYGON hp) {
	CHECK_HP(hp, "Out of range polygon handle (21)");
	assert(Polys[hp] != NULL);

	Poly ptp(LockMem(pHandle), Polys[hp]->pIndex);

	return (int)FROM_LE_32(ptp.zFactor);
}

int numNodes(HPOLYGON hp) {
	CHECK_HP(hp, "Out of range polygon handle (22)");
	assert(Polys[hp] != NULL);

	Poly ptp(LockMem(pHandle), Polys[hp]->pIndex);

	return ptp.getNodecount();
}

// *************************************************************************
//
// Code concerned with killing and reviving TAG and EXIT polygons.
// And code to enable this information to be saved and restored.
//
// *************************************************************************

struct TAGSTATE	{
	int tid;
	bool enabled;
};

#define MAX_SCENES	256
#define MAX_TAGS	2048
#define MAX_EXITS	512

static struct {
	SCNHANDLE sid;
	int	nooftags;
	int	offset;
} SceneTags[MAX_SCENES], SceneExits[MAX_SCENES];

static TAGSTATE TagStates[MAX_TAGS];
static TAGSTATE ExitStates[MAX_EXITS];

static int nextfreeT = 0, numScenesT = 0;
static int nextfreeE = 0, numScenesE = 0;

static int currentTScene = 0;
static int currentEScene = 0;

bool deadPolys[MAX_POLY];	// Currently just for dead blocks

void RebootDeadTags() {
	nextfreeT = numScenesT = 0;
	nextfreeE = numScenesE = 0;

	memset(SceneTags, 0, sizeof(SceneTags));
	memset(SceneExits, 0, sizeof(SceneExits));
	memset(TagStates, 0, sizeof(TagStates));
	memset(ExitStates, 0, sizeof(ExitStates));
	memset(deadPolys, 0, sizeof(deadPolys));
}

/**
 * (Un)serialize the dead tag and exit data for save/restore game.
 */
void syncPolyInfo(Common::Serializer &s) {
	int i;

	for (i = 0; i < MAX_SCENES; i++) {
		s.syncAsUint32LE(SceneTags[i].sid);
		s.syncAsSint32LE(SceneTags[i].nooftags);
		s.syncAsSint32LE(SceneTags[i].offset);
	}

	for (i = 0; i < MAX_SCENES; i++) {
		s.syncAsUint32LE(SceneExits[i].sid);
		s.syncAsSint32LE(SceneExits[i].nooftags);
		s.syncAsSint32LE(SceneExits[i].offset);
	}

	for (i = 0; i < MAX_TAGS; i++) {
		s.syncAsUint32LE(TagStates[i].tid);
		s.syncAsSint32LE(TagStates[i].enabled);
	}

	for (i = 0; i < MAX_EXITS; i++) {
		s.syncAsUint32LE(ExitStates[i].tid);
		s.syncAsSint32LE(ExitStates[i].enabled);
	}

	s.syncAsSint32LE(nextfreeT);
	s.syncAsSint32LE(numScenesT);
	s.syncAsSint32LE(nextfreeE);
	s.syncAsSint32LE(numScenesE);
}

/**
 * This is all totally different to the way the rest of the way polygon
 * data is stored and restored, more specifically, different to how dead
 * tags and exits are handled, because of the piecemeal design-by-just-
 * thought-of-this approach employed.
 */

void SaveDeadPolys(bool *sdp) {
	assert(!TinselV2);
	memcpy(sdp, deadPolys, MAX_POLY*sizeof(bool));
}

void RestoreDeadPolys(bool *sdp) {
	assert(!TinselV2);
	memcpy(deadPolys, sdp, MAX_POLY*sizeof(bool));
}

void SavePolygonStuff(POLY_VOLATILE *sps) {
	assert(TinselV2);
	memcpy(sps, volatileStuff, MAX_POLY*sizeof(POLY_VOLATILE));
}

void RestorePolygonStuff(POLY_VOLATILE *sps) {
	assert(TinselV2);
	memcpy(volatileStuff, sps, MAX_POLY*sizeof(POLY_VOLATILE));
}


/**
 * Scan for a given polygon
 */
static HPOLYGON FindPolygon(PTYPE type, int id) {

	for (int i = 0; i <= MAX_POLY; i++) {
		if (Polys[i] && Polys[i]->polyType == type && Polys[i]->polyID == id) {
			// Found it
			return i;
		}
	}

	// Not found
	return NOPOLY;
}

HPOLYGON FirstPathPoly() {
	for (int i = 0; i < noofPolys; i++) {
		if (Polys[i]->polyType == PATH)
			return i;
	}
	error("FirstPathPoly() - no PATH polygons");
	return NOPOLY;	// for compilers that don't support NORETURN
}

HPOLYGON GetPolyHandle(int i) {
	assert(i >= 0 && i <= MAX_POLY);

	return (Polys[i] != NULL) ? i : NOPOLY;
}

// **************************************************************************
//
// Code called to initialize or wrap up a scene:
//
// **************************************************************************

/**
 * Called at the start of a scene, when all polygons have been
 * initialized, to work out which paths are adjacent to which.
 */
static int DistinctCorners(HPOLYGON hp1, HPOLYGON hp2) {
	const POLYGON *pp1, *pp2;
	int	i, j;
	int	retval = 0;

	CHECK_HP(hp1, "Out of range polygon handle (23)");
	CHECK_HP(hp2, "Out of range polygon handle (24)");
	pp1 = Polys[hp1];
	pp2 = Polys[hp2];

	// Work out (how many of p1's corners is in p2) + (how many of p2's corners is in p1)
	for (i = 0; i < 4; i++) {
		if (IsInPolygon(pp1->cx[i], pp1->cy[i], hp2))
			retval++;
		if (IsInPolygon(pp2->cx[i], pp2->cy[i], hp1))
			retval++;
	}

	// Common corners only count once
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			if (pp1->cx[i] == pp2->cx[j] && pp1->cy[i] == pp2->cy[j])
				retval--;
		}
	}
	return retval;
}

/**
 * Returns true if the two paths are on the same level
 */
static bool MatchingLevels(PPOLYGON p1, PPOLYGON p2) {
	byte *pps = LockMem(pHandle);		// All polygons
	Poly pp1(pps, p1->pIndex);	// This polygon 1
	Poly pp2(pps, p2->pIndex);	// This polygon 2

	assert((int32)FROM_LE_32(pp1.level1) <= (int32)FROM_LE_32(pp1.level2));
	assert((int32)FROM_LE_32(pp2.level1) <= (int32)FROM_LE_32(pp2.level2));

	for (int pl = (int32)FROM_LE_32(pp1.level1); pl <= (int32)FROM_LE_32(pp1.level2); pl++) {
		if (pl >= (int32)FROM_LE_32(pp2.level1) && pl <= (int32)FROM_LE_32(pp2.level2))
			return true;
	}

	return false;
}

static void SetPathAdjacencies() {
	POLYGON *p1, *p2;		// Polygon pointers
	int i1, i2;

	// Reset them all
	for (i1 = 0; i1 < noofPolys; i1++)
		memset(Polys[i1]->adjpaths, 0, MAXADJ * sizeof(PPOLYGON));

	// For each polygon..
	for (i1 = 0; i1 < MAX_POLY-1; i1++) {
		// Get polygon, but only carry on if it's a path
		p1 = Polys[i1];
		if (!p1 || p1->polyType != PATH)
			continue;

		// For each subsequent polygon..
		for (i2 = i1 + 1; i2 < MAX_POLY; i2++) {
			// Get polygon, but only carry on if it's a path
			p2 = Polys[i2];
			if (!p2 || p2->polyType != PATH)
				continue;

			// Must be on the same level
			if (TinselV2 && !MatchingLevels(p1, p2))
				continue;

			int j = DistinctCorners(i1, i2);

			if (j >= 2) {
				// Paths are adjacent
				for (j = 0; j < MAXADJ; j++)
					if (p1->adjpaths[j] == NULL) {
						p1->adjpaths[j] = p2;
						break;
					}
#ifdef DEBUG
				if (j > highestYet)
					highestYet = j;
#endif
				assert(j < MAXADJ); // Number of adjacent paths limit
				for (j = 0; j < MAXADJ; j++) {
					if (p2->adjpaths[j] == NULL) {
						p2->adjpaths[j] = p1;
						break;
					}
				}
#ifdef DEBUG
				if (j > highestYet)
					highestYet = j;
#endif
				assert(j < MAXADJ); // Number of adjacent paths limit
			}
		}
	}
}

/**
 * Ensure NPATH nodes are not inside another PATH/NPATH polygon.
 * Only bother with end nodes for now.
 */
#ifdef DEBUG
void CheckNPathIntegrity() {
	uint8		*pps;	// Compiled polygon data
	const POLYGON *rp;	// Run-time polygon structure
	HPOLYGON	hp;
	int		i, j;	// Loop counters
	int		n;	// Last node in current path

	pps = LockMem(pHandle);		// All polygons

	for (i = 0; i < MAX_POLY; i++) {		// For each polygon..
		rp = Polys[i];
		if (rp && rp->polyType == PATH && rp->subtype == NODE) { //...if it's a node path
			// Get compiled polygon structure
			const Poly cp(pps, rp->pIndex);	// This polygon

			n = cp.getNodecount() - 1;		// Last node
			assert(n >= 1); // Node paths must have at least 2 nodes

			hp = PolygonIndex(rp);
			for (j = 0; j <= n; j++) {
				if (!IsInPolygon(cp.getNodeX(j), cp.getNodeY(j), hp)) {
					sprintf(TextBufferAddr(), "Node (%d, %d) is not in its own path (starting (%d, %d))",
						 cp.getNodeX(j), cp.getNodeY(j), rp->cx[0], rp->cy[0]);
					error(TextBufferAddr());
				}
			}

			// Check end nodes are not in adjacent path
			for (j = 0; j < MAXADJ; j++) {	// For each adjacent path
				if (rp->adjpaths[j] == NULL)
					break;

				if (IsInPolygon(cp.getNodeX(0), cp.getNodeY(0), PolygonIndex(rp->adjpaths[j]))) {
					sprintf(TextBufferAddr(), "Node (%d, %d) is in another path (starting (%d, %d))",
						 cp.getNodeX(0), cp.getNodeY(0), rp->adjpaths[j]->cx[0], rp->adjpaths[j]->cy[0]);
					error(TextBufferAddr());
				}
				if (IsInPolygon(cp.getNodeX(n), cp.getNodeY(n), PolygonIndex(rp->adjpaths[j]))) {
					sprintf(TextBufferAddr(), "Node (%d, %d) is in another path (starting (%d, %d))",
						 cp.getNodeX(n), cp.getNodeY(n), rp->adjpaths[j]->cx[0], rp->adjpaths[j]->cy[0]);
					error(TextBufferAddr());
				}
			}
		}
	}
}
#endif

/**
 * Called at the start of a scene, nobbles TAG polygons which should be dead.
 */
static void SetExBlocks() {
	for (int i = 0; i < MAX_POLY; i++) {
		if (deadPolys[i]) {
			if (Polys[i] && Polys[i]->polyType == BLOCK)
				Polys[i]->polyType = EX_BLOCK;
#ifdef DEBUG
			else
				error("Impossible message");
#endif
		}
	}
}

/**
 * Called at the start of a scene, nobbles TAG polygons which should be dead.
 */
static void SetExTags(SCNHANDLE ph) {
	int i, j;
	TAGSTATE *pts;

	for (i = 0; i < numScenesT; i++) {
		if (SceneTags[i].sid == ph) {
			currentTScene = i;

			pts = &TagStates[SceneTags[i].offset];
			for (j = 0; j < SceneTags[i].nooftags; j++, pts++) {
				if (!pts->enabled)
					DisableTag(Common::nullContext, pts->tid);
			}
			return;
		}
	}

	i = numScenesT++;
	currentTScene = i;
	assert(numScenesT < MAX_SCENES); // Dead tag remembering: scene limit

	SceneTags[i].sid = ph;
	SceneTags[i].offset = nextfreeT;
	SceneTags[i].nooftags = 0;

	for (j = 0; j < MAX_POLY; j++) {
		if (Polys[j] && Polys[j]->polyType == TAG) {
			TagStates[nextfreeT].tid = Polys[j]->polyID;
			TagStates[nextfreeT].enabled = true;
			nextfreeT++;
			assert(nextfreeT < MAX_TAGS); // Dead tag remembering: tag limit
			SceneTags[i].nooftags++;
		}
	}
}

/**
 * Called at the start of a scene, nobbles EXIT polygons which should be dead.
 */
static void SetExExits(SCNHANDLE ph) {
	TAGSTATE *pts;
	int i, j;

	for (i = 0; i < numScenesE; i++) {
		if (SceneExits[i].sid == ph) {
			currentEScene = i;

			pts = &ExitStates[SceneExits[i].offset];
			for (j = 0; j < SceneExits[i].nooftags; j++, pts++) {
				if (!pts->enabled)
					DisableExit(pts->tid);
			}
			return;
		}
	}

	i = numScenesE++;
	currentEScene = i;
	assert(numScenesE < MAX_SCENES); // Dead exit remembering: scene limit

	SceneExits[i].sid = ph;
	SceneExits[i].offset = nextfreeE;
	SceneExits[i].nooftags = 0;

	for (j = 0; j < MAX_POLY; j++) {
		if (Polys[j] && Polys[j]->polyType == EXIT) {
			ExitStates[nextfreeE].tid = Polys[j]->polyID;
			ExitStates[nextfreeE].enabled = true;
			nextfreeE++;
			assert(nextfreeE < MAX_EXITS); // Dead exit remembering: exit limit
			SceneExits[i].nooftags++;
		}
	}
}

/**
 * Works out some fixed numbers for a polygon.
 */
static void FiddlyBit(POLYGON *p) {
	int	t1, t2;		// General purpose temp. variables

	// Enclosing external rectangle
	t1 = MAX(p->cx[0], p->cx[1]);
	t2 = MAX(p->cx[2], p->cx[3]);
	p->pright = MAX(t1, t2);

	t1 = MIN(p->cx[0], p->cx[1]);
	t2 = MIN(p->cx[2], p->cx[3]);
	p->pleft = MIN(t1, t2);

	t1 = MAX(p->cy[0], p->cy[1]);
	t2 = MAX(p->cy[2], p->cy[3]);
	p->pbottom = MAX(t1, t2);

	t1 = MIN(p->cy[0], p->cy[1]);
	t2 = MIN(p->cy[2], p->cy[3]);
	p->ptop = MIN(t1, t2);

	// Rectangles enclosing each side and each side's magic numbers
	for (t1 = 0; t1 < 4; t1++) {
		p->lright[t1]   = MAX(p->cx[t1], p->cx[(t1+1)%4]);
		p->lleft[t1]    = MIN(p->cx[t1], p->cx[(t1+1)%4]);

		p->ltop[t1]     = MIN(p->cy[t1], p->cy[(t1+1)%4]);
		p->lbottom[t1]  = MAX(p->cy[t1], p->cy[(t1+1)%4]);

		p->a[t1] = p->cy[t1] - p->cy[(t1+1)%4];
		p->b[t1] = p->cx[(t1+1)%4] - p->cx[t1];
		p->c[t1] = (long)p->cy[t1]*p->cx[(t1+1)%4] - (long)p->cx[t1]*p->cy[(t1+1)%4];
	}
}

/**
 * Allocate a POLYGON structure and reset it to default values
 */
static PPOLYGON GetPolyEntry() {
	int i;		// Loop counter
	PPOLYGON p;

	for (i = 0; i < MaxPolys; i++) {
		if (!Polys[i]) {
			p = Polys[i] = &Polygons[i];

			// What the hell, just clear it all out - it's safer
			memset(p, 0, sizeof(POLYGON));

			return p;
		}
	}

	error("Exceeded MaxPolys");
}

/**
 * Variation of  GetPolyEntry from Tinsel 1 that splits up getting a new
 * polygon structure from initializing it
 */
static PPOLYGON CommonInits(PTYPE polyType, int pno, const Poly &ptp, bool bRestart) {
	int i;
	HPOLYGON hp;
	PPOLYGON p = GetPolyEntry();	// Obtain a slot

	p->polyType = polyType;			// Polygon type
	p->pIndex = pno;

	for (i = 0; i < 4; i++) {		// Polygon definition
		p->cx[i] = (short)FROM_LE_32(ptp.x[i]);
		p->cy[i] = (short)FROM_LE_32(ptp.y[i]);
	}

	if (!bRestart) {
		hp = PolygonIndex(p);
		volatileStuff[hp].xoff = (short)FROM_LE_32(ptp.xoff);
		volatileStuff[hp].yoff = (short)FROM_LE_32(ptp.yoff);
	}

	p->polyID = FROM_LE_32(ptp.id);			// Identifier

	FiddlyBit(p);

	return p;
}

/**
 * Calculate a point approximating to the center of a polygon.
 * Not very sophisticated.
 */
static void PseudoCenter(POLYGON *p) {
	p->pcenterx = (p->cx[0] + p->cx[1] + p->cx[2] + p->cx[3])/4;
	p->pcentery = (p->cy[0] + p->cy[1] + p->cy[2] + p->cy[3])/4;

	if (!IsInPolygon(p->pcenterx, p->pcentery, PolygonIndex(p))) {
		int i, top = 0, bot = 0;

		for (i = p->ptop; i <= p->pbottom; i++) {
			if (IsInPolygon(p->pcenterx, i, PolygonIndex(p))) {
				top = i;
				break;
			}
		}
		for (i = p->pbottom; i >= p->ptop; i--) {
			if (IsInPolygon(p->pcenterx, i, PolygonIndex(p))) {
				bot = i;
				break;
			}
		}
		p->pcenterx = (top+bot)/2;
	}
#ifdef DEBUG
	//	assert(IsInPolygon(p->pcenterx, p->pcentery, PolygonIndex(p)));  // Pseudo-center is not in path
	if (!IsInPolygon(p->pcenterx, p->pcentery, PolygonIndex(p))) {
		sprintf(TextBufferAddr(), "Pseudo-center is not in path (starting (%d, %d)) - polygon reversed?",
			p->cx[0], p->cy[0]);
		error(TextBufferAddr());
	}
#endif
}

/**
 * Initialize an EXIT polygon.
 */
static void InitExit(const Poly &ptp, int pno, bool bRestart) {
	CommonInits(EXIT, pno, ptp, bRestart);
}

/**
 * Initialize a PATH or NPATH polygon.
 */
static void InitPath(const Poly &ptp, bool NodePath, int pno, bool bRestart) {
	PPOLYGON p = CommonInits(PATH, pno, ptp, bRestart);

	p->subtype = NodePath ? NODE : NORMAL;

	PseudoCenter(p);
}


/**
 * Initialize a BLOCKING polygon.
 */
static void InitBlock(const Poly &ptp, int pno, bool bRestart) {
	CommonInits(BLOCK, pno, ptp, bRestart);
}

/**
 * Initialize an extra BLOCKING polygon related to a moving actor.
 * The width of the polygon depends on the width of the actor which is
 * trying to walk through the actor you first thought of.
 * This is for dynamic blocking.
 */
HPOLYGON InitExtraBlock(PMOVER ca, PMOVER ta) {
	int	caX, caY;	// Calling actor co-ords
	int	taX, taY;	// Test actor co-ords
	int	left, right;

	GetMoverPosition(ca, &caX, &caY);	// Calling actor co-ords
	GetMoverPosition(ta, &taX, &taY);	// Test actor co-ords

	left = GetMoverLeft(ta) - (GetMoverRight(ca) - caX);
	right = GetMoverRight(ta) + (caX - GetMoverLeft(ca));

	memset(&extraBlock, 0, sizeof(extraBlock));

	// The 3s on the y co-ordinates used to be 10s
	extraBlock.cx[0] = (short)(left - 2);
	extraBlock.cy[0] = (short)(taY - 3);
	extraBlock.cx[1] = (short)(right + 2);
	extraBlock.cy[1] = (short)(taY - 3);
	extraBlock.cx[2] = (short)(right + 2);
	extraBlock.cy[2] = (short)(taY + 3);
	extraBlock.cx[3] = (short)(left - 2);
	extraBlock.cy[3] = (short)(taY + 3);

	FiddlyBit(&extraBlock);		// Is this necessary?

	Polys[MAX_POLY] = &extraBlock;
	return MAX_POLY;
}

/**
 * Initialize an EFFECT polygon.
 */
static void InitEffect(const Poly &ptp, int pno, bool bRestart) {
	CommonInits(EFFECT, pno, ptp, bRestart);
}


/**
 * Initialize a REFER polygon.
 */
static void InitRefer(const Poly &ptp, int pno, bool bRestart) {
	PPOLYGON p = CommonInits(REFER, pno, ptp, bRestart);

	p->subtype = FROM_LE_32(ptp.reftype);	// Refer type
}


/**
 * Initialize a TAG polygon.
 */
static void InitTag(const Poly &ptp, int pno, bool bRestart) {
	CommonInits(TAG, pno, ptp, bRestart);
}

/**
 * Called at the restart of a scene, nobbles polygons which are dead.
 */
static void KillDeadPolygons() {
	int i;

	for (i = 0; i < MAX_POLY; i++) {
		if (volatileStuff[i].bDead) {
			assert(Polys[i]);

			switch (Polys[i]->polyType) {
			case BLOCK:
				Polys[i]->polyType = EX_BLOCK;
				break;

			case EFFECT:
				Polys[i]->polyType = EX_EFFECT;
				break;

			case REFER:
				Polys[i]->polyType = EX_REFER;
				break;

			case PATH:
				Polys[i]->polyType = EX_PATH;
				break;

			case TAG:
				Polys[i]->polyType = EX_TAG;
				break;

			default:
				error("Impossible message");
			}
		}
	}
}

/**
 * Called at the start of a scene to initialize the polys in that scene.
 */
void InitPolygons(SCNHANDLE ph, int numPoly, bool bRestart) {
	pHandle = ph;
	noofPolys = numPoly;

	if (Polygons == NULL) {
		// first time - allocate memory for process list
		Polygons = (POLYGON *)calloc(MaxPolys, sizeof(POLYGON));

		// make sure memory allocated
		if (Polygons == NULL) {
			error("Cannot allocate memory for polygon data");
		}
	}

	if (numPoly == 0)
		return;

	for (int i = 0; i < noofPolys; i++)	{
		if (Polys[i]) {
			Polys[i]->pointState = PS_NOT_POINTING;
			Polys[i] = NULL;
		}
	}

	memset(RoutePaths, 0, sizeof(RoutePaths));

	if (!bRestart) {
		if (TinselV2)
			memset(volatileStuff, 0, sizeof(volatileStuff));
		else
			memset(deadPolys, 0, sizeof(deadPolys));
	}

	if (numPoly > 0) {
		Poly ptp(LockMem(ph));

		for (int i = 0; i < numPoly; ++i, ++ptp) {
			switch (ptp.getType()) {
			case POLY_PATH:
				InitPath(ptp, false, i, bRestart);
				break;

			case POLY_NPATH:
				InitPath(ptp, true, i, bRestart);
				break;

			case POLY_BLOCK:
				InitBlock(ptp, i, bRestart);
				break;

			case POLY_REFER:
				InitRefer(ptp, i, bRestart);
				break;

			case POLY_EFFECT:
				InitEffect(ptp, i, bRestart);
				break;

			case POLY_EXIT:
				InitExit(ptp, i, bRestart);
				break;

			case POLY_TAG:
				InitTag(ptp, i, bRestart);
				break;

			default:
				error("Unknown polygon type");
			}
		}
	}

	if (!TinselV2) {
		SetPathAdjacencies();		// Paths need to know the facts
#ifdef DEBUG
		CheckNPathIntegrity();
#endif

		SetExTags(ph);			// Some tags may have been killed
		SetExExits(ph);			// Some exits may have been killed

		if (bRestart)
			SetExBlocks();		// Some blocks may have been killed
	} else {
		if (bRestart) {
			// Some may have been killed if this is a restore
			KillDeadPolygons();
		} else {
			for (int i = numPoly - 1; i >= 0; i--) {
				if (Polys[i]->polyType == TAG) {
					PolygonEvent(Common::nullContext, i, STARTUP, 0, false, 0);
				}
			}
		}

		// Paths need to know the facts
		SetPathAdjacencies();
	}
}

/**
 * Called at the end of a scene to ditch all polygons.
 */
void DropPolygons() {
	pathsOnRoute = 0;
	memset(RoutePaths, 0, sizeof(RoutePaths));
	RouteEnd = NULL;

	for (int i = 0; i < noofPolys; i++)	{
		if (Polys[i]) {
			Polys[i]->pointState = PS_NOT_POINTING;
			Polys[i] = NULL;
		}
	}
	noofPolys = 0;
	free(Polygons);
	Polygons = NULL;
}



PTYPE PolyType(HPOLYGON hp) {
	CHECK_HP(hp, "Out of range polygon handle (25)");

	return Polys[hp]->polyType;
}

int PolySubtype(HPOLYGON hp) {
	CHECK_HP(hp, "Out of range polygon handle (26)");

	return Polys[hp]->subtype;
}

int PolyCenterX(HPOLYGON hp) {
	CHECK_HP(hp, "Out of range polygon handle (27)");

	return Polys[hp]->pcenterx;
}

int PolyCenterY(HPOLYGON hp) {
	CHECK_HP(hp, "Out of range polygon handle (28)");

	return Polys[hp]->pcentery;
}

int PolyCornerX(HPOLYGON hp, int n) {
	CHECK_HP(hp, "Out of range polygon handle (29)");

	return Polys[hp]->cx[n];
}

int PolyCornerY(HPOLYGON hp, int n) {
	CHECK_HP(hp, "Out of range polygon handle (30)");

	return Polys[hp]->cy[n];
}

PSTATE PolyPointState(HPOLYGON hp) {
	CHECK_HP(hp, "Out of range polygon handle (31)");

	return Polys[hp]->pointState;
}

TSTATE PolyTagState(HPOLYGON hp) {
	CHECK_HP(hp, "Out of range polygon handle (32)");

	return Polys[hp]->tagState;
}

void SetPolyPointState(HPOLYGON hp, PSTATE ps) {
	CHECK_HP(hp, "Out of range polygon handle (34)");

	Polys[hp]->pointState = ps;
}

void SetPolyTagState(HPOLYGON hp, TSTATE ts) {
	CHECK_HP(hp, "Out of range polygon handle (35)");

	Polys[hp]->tagState = ts;
}

void SetPolyTagHandle(HPOLYGON hp, SCNHANDLE th) {
	CHECK_HP(hp, "Out of range polygon handle (36)");

	Polys[hp]->hOverrideTag = th;
}

void MaxPolygons(int numPolys) {
	assert(numPolys <= MAX_POLY);

	MaxPolys = numPolys;
}

/**
 * Get polygon's associated node.
 * The one for WalkTag(), StandTag() etc.
 */
void GetPolyNode(HPOLYGON hp, int *pNodeX, int *pNodeY) {
	CHECK_HP(hp, "GetPolyNode(): Out of range polygon handle");

	Poly ptp(LockMem(pHandle), Polys[hp]->pIndex);

	// WORKAROUND: Invalid node adjustment for DW2 Cartwheel scene refer polygon
	if (TinselV2 && (pHandle == 0x74191900) && (hp == 8)) {
		*pNodeX = 480;
		*pNodeY = 408;
	} else {
		*pNodeX = FROM_LE_32(ptp.nodex);
		*pNodeY = FROM_LE_32(ptp.nodey);
	}

	if (TinselV2) {
		*pNodeX += volatileStuff[hp].xoff;
		*pNodeY += volatileStuff[hp].yoff;
	}
}

void SetPolyPointedTo(HPOLYGON hp, bool bPointedTo) {
	CHECK_HP(hp, "Out of range polygon handle (34)");

	if (bPointedTo)
		Polys[hp]->tagFlags |= POINTING;
	else
		Polys[hp]->tagFlags &= ~POINTING;
}

bool PolyIsPointedTo(HPOLYGON hp) {
	CHECK_HP(hp, "Out of range polygon handle (31)");

	if (TinselV2)
		return (Polys[hp]->tagFlags & POINTING);

	return PolyPointState(hp) == PS_POINTING;
}

void SetPolyTagWanted(HPOLYGON hp, bool bTagWanted, bool bCursor, SCNHANDLE hOverrideTag) {
	CHECK_HP(hp, "Out of range polygon handle (35)");

	if (bTagWanted) {
		Polys[hp]->tagFlags |= TAGWANTED;
		Polys[hp]->hOverrideTag = hOverrideTag;
	} else {
		Polys[hp]->tagFlags &= ~TAGWANTED;
		Polys[hp]->hOverrideTag = 0;
	}

	if (bCursor)
		Polys[hp]->tagFlags |= FOLLOWCURSOR;
	else
		Polys[hp]->tagFlags &= ~FOLLOWCURSOR;
}

bool PolyTagIsWanted(HPOLYGON hp) {
	CHECK_HP(hp, "Out of range polygon handle (32)");

	return (Polys[hp]->tagFlags & TAGWANTED);
}

bool PolyTagFollowsCursor(HPOLYGON hp) {
	CHECK_HP(hp, "Out of range polygon handle (36)");

	return (Polys[hp]->tagFlags & FOLLOWCURSOR);
}

SCNHANDLE GetPolyTagHandle(HPOLYGON hp) {
	CHECK_HP(hp, "Out of range polygon handle (33)");

	return Polys[hp]->hOverrideTag;
}

bool IsTagPolygon(int tagno) {
	return (FindPolygon(TAG, tagno) != NOPOLY || FindPolygon(EX_TAG, tagno) != NOPOLY);
}

int GetTagPolyId(HPOLYGON hp) {
	CHECK_HP(hp, "Out of range polygon handle (GetTagPolyId()");

	assert(Polys[hp]->polyType == TAG || Polys[hp]->polyType == EX_TAG);

	return Polys[hp]->polyID;
}

void GetPolyMidBottom(	HPOLYGON hp, int *pX, int *pY) {
	CHECK_HP(hp, "Out of range polygon handle (GetPolyMidBottom()");

	*pY = Polys[hp]->pbottom + volatileStuff[hp].yoff;
	*pX = (Polys[hp]->pleft + Polys[hp]->pright)/2 + volatileStuff[hp].xoff;
}

int PathCount() {
	int i, count;

	for (i = 0, count = 0; i < noofPolys; i++) {
		if (Polys[i]->polyType == PATH)
			count++;
	}

	return count;
}

/**
 * Convert a BLOCK to an EX_BLOCK poly
 */
void DisableBlock(int block) {
	int i = FindPolygon(BLOCK, block);

	if (i != NOPOLY) {
		Polys[i]->polyType = EX_BLOCK;
		volatileStuff[i].bDead = true;
	}
}

/**
 * Convert an EX_BLOCK to a BLOCK poly
 */
void EnableBlock(int block) {
	int i = FindPolygon(EX_BLOCK, block);

	if (i != NOPOLY) {
		Polys[i]->polyType = BLOCK;
		volatileStuff[i].bDead = false;
	}
}

/**
 * Convert an EFFECT to an EX_EFFECT poly
 */
void DisableEffect(int effect) {
	int i = FindPolygon(EFFECT, effect);

	if (i != NOPOLY) {
		Polys[i]->polyType = EX_EFFECT;
		volatileStuff[i].bDead = true;
	}
}

/**
 * Convert an EX_EFFECT to an EFFECT poly
 */
void EnableEffect(int effect) {
	int i = FindPolygon(EX_EFFECT, effect);

	if (i != NOPOLY) {
		Polys[i]->polyType = EFFECT;
		volatileStuff[i].bDead = false;
	}
}

/**
 * Convert a PATH to an EX_PATH poly
 */
void DisablePath(int path) {
	int i = FindPolygon(PATH, path);

	if (i != NOPOLY) {
		Polys[i]->polyType = EX_PATH;
		volatileStuff[i].bDead = true;

		// Paths need to know the new facts
		SetPathAdjacencies();
	}
}

/**
 * Convert a PATH to an EX_PATH poly
 */
void EnablePath(int path) {
	int i = FindPolygon(EX_PATH, path);

	if (i != NOPOLY) {
		Polys[i]->polyType = PATH;
		volatileStuff[i].bDead = false;

		// Paths need to know the new facts
		SetPathAdjacencies();
	}
}

/**
 * Convert a REFER to an EX_REFER poly
 */
void DisableRefer(int refer) {
	int i = FindPolygon(REFER, refer);

	if (i != NOPOLY) {
		Polys[i]->polyType = EX_REFER;
		volatileStuff[i].bDead = true;
	}
}

/**
 * Convert a REFER to an EX_REFER poly
 */
void EnableRefer(int refer) {
	int i = FindPolygon(EX_REFER, refer);

	if (i != NOPOLY) {
		Polys[i]->polyType = REFER;
		volatileStuff[i].bDead = false;
	}
}

/**
 * Convert an EX_TAG to a TAG poly.
 */
void EnableTag(CORO_PARAM, int tag) {
	CORO_BEGIN_CONTEXT;
		int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if ((_ctx->i = FindPolygon(EX_TAG, tag)) != NOPOLY) {
		Polys[_ctx->i]->polyType = TAG;
		volatileStuff[_ctx->i].bDead = false;

		if (TinselV2)
			CORO_INVOKE_ARGS(PolygonEvent, (CORO_SUBCTX, _ctx->i, SHOWEVENT, 0, true, 0));
	} else if ((_ctx->i = FindPolygon(TAG, tag)) != NOPOLY) {
		if (TinselV2)
			CORO_INVOKE_ARGS(PolygonEvent, (CORO_SUBCTX, _ctx->i, SHOWEVENT, 0, true, 0));
	}

	if (!TinselV2) {
		TAGSTATE *pts = &TagStates[SceneTags[currentTScene].offset];
		for (int j = 0; j < SceneTags[currentTScene].nooftags; j++, pts++) {
			if (pts->tid == tag) {
				pts->enabled = true;
				break;
			}
		}
	}

	CORO_END_CODE;
}

/**
 * Convert an EX_EXIT to a EXIT poly.
 */
void EnableExit(int exitno) {
	for (int i = 0; i < MAX_POLY; i++) {
		if (Polys[i] && Polys[i]->polyType == EX_EXIT && Polys[i]->polyID == exitno) {
			Polys[i]->polyType = EXIT;
		}
	}

	TAGSTATE *pts;
	pts = &ExitStates[SceneExits[currentEScene].offset];
	for (int j = 0; j < SceneExits[currentEScene].nooftags; j++, pts++) {
		if (pts->tid == exitno) {
			pts->enabled = true;
			break;
		}
	}
}

/**
 * Move a polygon relative to current offset.
 */
void MovePolygon(PTYPE ptype, int id, int x, int y) {
	int i = FindPolygon(ptype, id);

	// If not found, try its dead equivalent
	if (i == NOPOLY) {
		switch (ptype) {
		case TAG:
			ptype = EX_TAG;
			break;
		default:
			break;
		}

		i = FindPolygon(ptype, id);
	}

	if (i != NOPOLY) {
		volatileStuff[i].xoff += (short)x;
		volatileStuff[i].yoff += (short)y;
	}
}

/**
 * Move a polygon relative to absolute offset.
 */
void MovePolygonTo(PTYPE ptype, int id, int x, int y) {
	int i = FindPolygon(ptype, id);

	// If not found, try its dead equivalent
	if (i == NOPOLY) {
		switch (ptype) {
		case TAG:
			ptype = EX_TAG;
			break;
		default:
			break;
		}

		i = FindPolygon(ptype, id);
	}

	if (i != NOPOLY) {
		volatileStuff[i].xoff = (short)x;
		volatileStuff[i].yoff = (short)y;
	}
}


/**
 * Convert tag number to polygon handle.
 */
HPOLYGON GetTagHandle(int tagno) {
	int i = FindPolygon(TAG, tagno);

	if (i == NOPOLY)
		i = FindPolygon(EX_TAG, tagno);

	assert(i != NOPOLY);

	return GetPolyHandle(i);
}

/**
 * Convert a TAG to an EX_TAG poly.
 */
void DisableTag(CORO_PARAM, int tag) {
	CORO_BEGIN_CONTEXT;
		int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if ((_ctx->i = FindPolygon(TAG, tag)) != NOPOLY) {
		Polys[_ctx->i]->polyType = EX_TAG;
		Polys[_ctx->i]->tagFlags = 0;
		Polys[_ctx->i]->tagState = TAG_OFF;
		Polys[_ctx->i]->pointState = PS_NOT_POINTING;

		volatileStuff[_ctx->i].bDead = true;

		if (TinselV2)
			CORO_INVOKE_ARGS(PolygonEvent, (CORO_SUBCTX, _ctx->i, HIDEEVENT, 0, true, 0));
	} else if ((_ctx->i = FindPolygon(EX_TAG, tag)) != NOPOLY) {
		if (TinselV2)
			CORO_INVOKE_ARGS(PolygonEvent, (CORO_SUBCTX, _ctx->i, HIDEEVENT, 0, true, 0));
	}

	if (!TinselV2) {
		TAGSTATE *pts = &TagStates[SceneTags[currentTScene].offset];
		for (int j = 0; j < SceneTags[currentTScene].nooftags; j++, pts++) {
			if (pts->tid == tag) {
				pts->enabled = false;
				break;
			}
		}
	}

	CORO_END_CODE;
}

/**
 * Convert a EXIT to an EX_EXIT poly.
 */
void DisableExit(int exitno) {
	TAGSTATE *pts;

	for (int i = 0; i < MAX_POLY; i++) {
		if (Polys[i] && Polys[i]->polyType == EXIT && Polys[i]->polyID == exitno) {
			Polys[i]->polyType = EX_EXIT;
			Polys[i]->tagState = TAG_OFF;
			Polys[i]->pointState = PS_NOT_POINTING;
		}
	}

	pts = &ExitStates[SceneExits[currentEScene].offset];
	for (int j = 0; j < SceneExits[currentEScene].nooftags; j++, pts++) {
		if (pts->tid == exitno) {
			pts->enabled = false;
			break;
		}
	}
}

} // End of namespace Tinsel
