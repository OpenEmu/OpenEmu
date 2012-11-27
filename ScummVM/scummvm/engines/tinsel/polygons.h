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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Definition of POLYGON structure and functions in POLYGONS.C
 */

#ifndef TINSEL_POLYGONS_H     // prevent multiple includes
#define TINSEL_POLYGONS_H

#include "tinsel/dw.h"	// for SCNHANDLE
#include "tinsel/scene.h"	// for PPOLY and REEL

namespace Tinsel {


// Polygon Types
enum PTYPE {
	// Tinsel 2 Polygon type list
	TEST,
	BLOCK, EFFECT, PATH, REFER, TAG,
	EX_BLOCK, EX_EFFECT, EX_PATH, EX_REFER, EX_TAG,
	// Extra polygon types from Tinsel v1
	EXIT, EX_EXIT
};

// subtype
enum {
	NORMAL = 0,
	NODE   = 1       // For paths
};

// tagFlags
enum {
	POINTING = 0x01,
	TAGWANTED = 0x02,
	FOLLOWCURSOR = 0x04
};

// tagState
enum TSTATE {
	TAG_OFF, TAG_ON
};

// pointState
enum PSTATE {
	PS_NO_POINT, PS_NOT_POINTING, PS_POINTING
};


enum {
	NOPOLY = -1
};

struct POLY_VOLATILE {
	bool bDead;
	short xoff, yoff;	// Polygon offset
};

/*-------------------------------------------------------------------------*/

bool IsInPolygon(int xt, int yt, HPOLYGON p);
HPOLYGON InPolygon(int xt, int yt, PTYPE type);
void BlockingCorner(HPOLYGON poly, int *x, int *y, int tarx, int tary);
void FindBestPoint(HPOLYGON path, int *x, int *y, int *line);
bool IsAdjacentPath(HPOLYGON path1, HPOLYGON path2);
HPOLYGON GetPathOnTheWay(HPOLYGON from, HPOLYGON to);
int NearestEndNode(HPOLYGON path, int x, int y);
int NearEndNode(HPOLYGON spath, HPOLYGON dpath);
int NearestNodeWithin(HPOLYGON npath, int x, int y);
void NearestCorner(int *x, int *y, HPOLYGON spath, HPOLYGON dpath);
bool IsPolyCorner(HPOLYGON hPath, int x, int y);
int GetScale(HPOLYGON path, int y);
int GetBrightness(HPOLYGON hPath, int y);
void getNpathNode(HPOLYGON npath, int node, int *px, int *py);
SCNHANDLE GetPolyFilm(HPOLYGON p);
void GetPolyNode(HPOLYGON hp, int *pNodeX, int *pNodeY);
SCNHANDLE GetPolyScript(HPOLYGON p);
REEL GetPolyReelType(HPOLYGON p);
int32 GetPolyZfactor(HPOLYGON p);
int numNodes(HPOLYGON pp);
void RebootDeadTags();
void DisableBlock(int block);
void EnableBlock(int block);
void DisableEffect(int effect);
void EnableEffect(int effect);
void DisablePath(int path);
void EnablePath(int path);
void DisableRefer(int refer);
void EnableRefer(int refer);
HPOLYGON GetTagHandle(int tagno);
void DisableTag(CORO_PARAM, int tag);
void EnableTag(CORO_PARAM, int tag);
void DisableExit(int exitno);
void EnableExit(int exitno);
HPOLYGON FirstPathPoly();
HPOLYGON GetPolyHandle(int i);
void InitPolygons(SCNHANDLE ph, int numPoly, bool bRestart);
void DropPolygons();


void SaveDeadPolys(bool *sdp);
void RestoreDeadPolys(bool *sdp);
void SavePolygonStuff(POLY_VOLATILE *sps);
void RestorePolygonStuff(POLY_VOLATILE *sps);

/*-------------------------------------------------------------------------*/

PTYPE PolyType(HPOLYGON hp);		// ->type
int PolySubtype(HPOLYGON hp);		// ->subtype
int PolyCenterX(HPOLYGON hp);		// ->pcenterx
int PolyCenterY(HPOLYGON hp);		// ->pcentery
int PolyCornerX(HPOLYGON hp, int n);	// ->cx[n]
int PolyCornerY(HPOLYGON hp, int n);	// ->cy[n]
PSTATE PolyPointState(HPOLYGON hp);	// ->pointState
TSTATE PolyTagState(HPOLYGON hp);	// ->tagState

void SetPolyPointState(HPOLYGON hp, PSTATE ps);	// ->pointState
void SetPolyTagState(HPOLYGON hp, TSTATE ts);	// ->tagState
void SetPolyTagHandle(HPOLYGON hp, SCNHANDLE th);// ->oTagHandle

void MaxPolygons(int maxPolys);

int GetTagPolyId(HPOLYGON hp);
void GetTagTag(HPOLYGON hp, SCNHANDLE *hTagText, int *tagX, int *tagY);
void SetPolyPointedTo(HPOLYGON hp, bool bPointedTo);
bool PolyIsPointedTo(HPOLYGON hp);
void SetPolyTagWanted(HPOLYGON hp, bool bTagWanted, bool bCursor, SCNHANDLE hOverrideTag);
bool PolyTagIsWanted(HPOLYGON hp);
bool PolyTagFollowsCursor(HPOLYGON hp);
SCNHANDLE GetPolyTagHandle(HPOLYGON hp);
bool IsTagPolygon(int tagno);
void GetPolyMidBottom(HPOLYGON hp, int *pX, int *pY);
int PathCount();
void MovePolygon(PTYPE ptype, int id, int x, int y);
void MovePolygonTo(PTYPE ptype, int id, int x, int y);

/*-------------------------------------------------------------------------*/

} // End of namespace Tinsel

#endif		/* TINSEL_POLYGONS_H */
