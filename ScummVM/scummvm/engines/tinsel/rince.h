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
 * Should really be called "moving actors.h"
 */

#ifndef TINSEL_RINCE_H	// prevent multiple includes
#define TINSEL_RINCE_H

#include "tinsel/anim.h"	// for ANIM
#include "tinsel/scene.h"	// for TFTYPE
#include "tinsel/tinsel.h"

namespace Tinsel {

struct OBJECT;

enum NPS {NOT_IN, GOING_UP, GOING_DOWN, LEAVING, ENTERING};

enum IND {NO_PROB, TRY_CENTER, TRY_CORNER, TRY_NEXTCORNER};

enum DIRECTION { LEFTREEL, RIGHTREEL, FORWARD, AWAY };

#define NUM_MAINSCALES	(TinselV2 ? 10 : 5)
#define NUM_AUXSCALES	5
#define TOTAL_SCALES	(NUM_MAINSCALES + NUM_AUXSCALES)
#define REQ_MAIN_SCALES		10
#define REQ_TOTAL_SCALES	15

#define BOGUS_BRIGHTNESS -1

struct MOVER {

	int		objX, objY;           /* Co-ordinates object  */

	int     targetX, targetY;
	int     ItargetX, ItargetY;     /* Intermediate destination */
	int     UtargetX, UtargetY;     /* Ultimate destination */

	HPOLYGON hIpath;
	HPOLYGON hUpath;
	HPOLYGON hCpath;

	bool over;
	int	walkNumber;

	IND	InDifficulty;

	/* For use in 'follow nodes' polygons   */
	HPOLYGON hFnpath;
	NPS	npstatus;
	int     line;

	int     Tline;                   // NEW

	// TODO: TagReelRunning may be the same as bSpecReel
	bool		bSpecReel;

	/* Used internally */
	DIRECTION	direction;		// Current reel
	int			scale;		// Current scale

	int			stepCount;		// Step count for walking reel synchronisation

	int			walkedFromX, walkedFromY;

	bool		bMoving;		// Set this to TRUE during a walk

	bool		bNoPath;
	bool		bIgPath;
	bool		bWalkReel;

	OBJECT		*actorObj;	// Actor's object
	ANIM		actorAnim;	// Actor's animation script

	SCNHANDLE	hLastFilm;	// } Used by AlterMover()
	SCNHANDLE	hPushedFilm;	// }

	int			actorID;
	int			actorToken;

	SCNHANDLE	walkReels[REQ_TOTAL_SCALES][4];
	SCNHANDLE	standReels[REQ_TOTAL_SCALES][4];
	SCNHANDLE	talkReels[REQ_TOTAL_SCALES][4];

	bool		bActive;

	int			SlowFactor;	// Slow down movement while hidden

	bool		bStop;

	/* NOTE: If effect polys can overlap, this needs improving */
	bool		bInEffect;

	Common::PROCESS		*pProc;

	// Discworld 2 specific fields
	int32		zOverride;
	bool		bHidden;
	int			brightness;	// Current brightness
	int			startColor;
	int			paletteLength;
	HPOLYGON	hRpath;		// Recent path
};
typedef MOVER *PMOVER;

struct MAINIT {
	int	X;
	int	Y;
	PMOVER	pMover;
};
typedef MAINIT *PMAINIT;

//---------------------------------------------------------------------------


void MoverProcessCreate(int X, int Y, int id, PMOVER pMover);


enum AR_FUNCTION { AR_NORMAL, AR_PUSHREEL, AR_POPREEL, AR_WALKREEL };

void StoreMoverPalette(PMOVER pMover, int startColor, int length);

void MoverBrightness(PMOVER pMover, int brightness);

MOVER *GetMover(int ano);
MOVER *RegisterMover(int ano);
void KillMover(PMOVER pMover);
MOVER *GetLiveMover(int index);

bool getMActorState(MOVER *psActor);
int GetMoverId(PMOVER pMover);
void SetMoverZ(PMOVER pMover, int y, uint32 zFactor);
void SetMoverZoverride(PMOVER pMover, uint32 zFactor);

void HideMover(PMOVER pMover, int sf = 0);
bool MoverHidden(PMOVER pMover);
bool MoverIs(PMOVER pMover);
bool MoverIsSWalking(PMOVER pMover);
bool MoverMoving(PMOVER pMover);
int GetWalkNumber(PMOVER pMover);
void UnHideMover(PMOVER pMover);
void DropMovers();
void PositionMover(PMOVER pMover, int x, int y);

void GetMoverPosition(PMOVER pMover, int *aniX, int *aniY);
void GetMoverMidTop(PMOVER pMover, int *aniX, int *aniY);
int GetMoverLeft(PMOVER pMover);
int GetMoverRight(PMOVER pMover);
int GetMoverTop(PMOVER pMover);
int GetMoverBottom(PMOVER pMover);

bool MoverIsInPolygon(PMOVER pMover, HPOLYGON hPoly);
void AlterMover(PMOVER pMover, SCNHANDLE film, AR_FUNCTION fn);
DIRECTION GetMoverDirection(PMOVER pMover);
int GetMoverScale(PMOVER pMover);
void SetMoverDirection(PMOVER pMover, DIRECTION dirn);
void SetMoverStanding(PMOVER pMover);
void SetMoverWalkReel(PMOVER pMover, DIRECTION reel, int scale, bool force);

PMOVER InMoverBlock(PMOVER pMover, int x, int y);

void RebootMovers();

bool IsMAinEffectPoly(int index);
void SetMoverInEffect(int index, bool tf);

void StopMover(PMOVER pMover);

/*----------------------------------------------------------------------*/

struct SAVED_MOVER {

	int		actorID;
	int		objX;
	int		objY;
	SCNHANDLE hLastfilm;

	SCNHANDLE	walkReels[REQ_TOTAL_SCALES][4];
	SCNHANDLE	standReels[REQ_TOTAL_SCALES][4];
	SCNHANDLE	talkReels[REQ_TOTAL_SCALES][4];

	bool	bActive;
	bool	bHidden;
	int		brightness;
	int		startColor;
	int		paletteLength;
};

void SaveMovers(SAVED_MOVER *sMoverInfo);
void RestoreAuxScales(SAVED_MOVER *sMoverInfo);

PMOVER NextMover(PMOVER pMover);

/*----------------------------------------------------------------------*/

enum {
	MAGICX	= -101,
	MAGICY	= -102
};

/*----------------------------------------------------------------------*/

} // End of namespace Tinsel

#endif /* TINSEL_RINCE_H */
