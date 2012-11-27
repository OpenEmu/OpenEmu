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
 * Prototypes of actor functions
 */

#ifndef TINSEL_ACTOR_H	// prevent multiple includes
#define TINSEL_ACTOR_H


#include "tinsel/dw.h"		// for SCNHANDLE
#include "tinsel/events.h"	// for TINSEL_EVENT
#include "tinsel/palette.h"	// for COLORREF

namespace Tinsel {

struct FREEL;
struct INT_CONTEXT;
struct MOVER;
struct OBJECT;

#define ACTORTAG_KEY 0x1000000

#define OTH_RELATEDACTOR	0x00000fff
#define OTH_RELATIVE		0x00001000
#define OTH_ABSOLUTE		0x00002000

/*----------------------------------------------------------------------*/

void RegisterActors(int num);
void FreeActors();
void SetLeadId(int rid);
int GetLeadId();
bool ActorIsGhost(int actor);
void StartTaggedActors(SCNHANDLE ah, int numActors, bool bRunScript);
void DropActors();		// No actor reels running
void DisableActor(int actor);
void EnableActor(int actor);
void Tag_Actor(int ano, SCNHANDLE tagtext, int tp);
void UnTagActor(int ano);
void ReTagActor(int ano);
int TagType(int ano);
bool actorAlive(int ano);
int32 actorMaskType(int ano);
void GetActorPos(int ano, int *x, int *y);
void SetActorPos(int ano, int x, int y);
void GetActorMidTop(int ano, int *x, int *y);
int GetActorLeft(int ano);
int GetActorRight(int ano);
int GetActorTop(int ano);
int GetActorBottom(int ano);
void ShowActor(CORO_PARAM, int ano);
void HideActor(CORO_PARAM, int ano);
bool ActorHidden(int ano);
bool HideMovingActor(int id, int sf);
void unHideMovingActor(int id);
void restoreMovement(int id);
void storeActorReel(int ano, const FREEL *reel, SCNHANDLE hFilm, OBJECT *pobj, int reelnum, int x, int y);
const FREEL *actorReel(int ano);
SCNHANDLE actorFilm(int ano);

void SetActorPlayFilm(int ano, SCNHANDLE hFilm);
SCNHANDLE GetActorPlayFilm(int ano);
void SetActorTalkFilm(int ano, SCNHANDLE hFilm);
SCNHANDLE GetActorTalkFilm(int ano);
void SetActorTalking(int ano, bool tf);
bool ActorIsTalking(int ano);
void SetActorLatestFilm(int ano, SCNHANDLE hFilm);
SCNHANDLE GetActorLatestFilm(int ano);

void UpdateActorEsc(int ano, bool escOn, int escEvent);
void UpdateActorEsc(int ano, int escEvent);
bool ActorEsc(int ano);
int ActorEev(int ano);
void StoreActorPos(int ano, int x, int y);
void StoreActorSteps(int ano, int steps);
int GetActorSteps(int ano);
void StoreActorZpos(int ano, int z, int column = -1);
int GetActorZpos(int ano, int column);
void IncLoopCount(int ano);
int GetLoopCount(int ano);
SCNHANDLE GetActorTag(int ano);
void FirstTaggedActor();
int NextTaggedActor();
int NextTaggedActor(int previous);
int AsetZPos(OBJECT *pObj, int y, int32 zFactor);
void SetMoverZ(MOVER *pMover, int y, int32 zFactor);
void ActorEvent(int ano, TINSEL_EVENT event, PLR_EVENT be);

void storeActorAttr(int ano, int r1, int g1, int b1);
COLORREF GetActorRGB(int ano);
void SetActorRGB(int ano, COLORREF color);
void SetActorZfactor(int ano, uint32 zFactor);
uint32 GetActorZfactor(int ano);

void setactorson();

void ActorsLife(int id, bool bAlive);

void dwEndActor(int ano);

void ActorEvent(CORO_PARAM, int ano, TINSEL_EVENT tEvent, bool bWait, int myEscape, bool *result = NULL);

void GetActorTagPortion(int ano, unsigned *top, unsigned *bottom, unsigned *left, unsigned *right);
SCNHANDLE GetActorTagHandle(int ano);
void SetActorPointedTo(int actor, bool bPointedTo);
bool ActorIsPointedTo(int actor);
void SetActorTagWanted(int actor, bool bTagWanted, bool bCursor, SCNHANDLE hOverrideTag);
bool ActorTagIsWanted(int actor);
bool InHotSpot(int ano, int curX, int curY);
int FrontTaggedActor();
void GetActorTagPos(int actor, int *pTagX, int *pTagY, bool bAbsolute);
bool IsTaggedActor(int actor);
void StoreActorPresFilm(int ano, SCNHANDLE hFilm, int x, int y);
SCNHANDLE GetActorPresFilm(int ano);
int GetActorFilmNumber(int ano);
void StoreActorReel(int actor, int column, OBJECT *pObj);
void NotPlayingReel(int actor, int filmNumber, int column);
bool ActorReelPlaying(int actor, int column);

/*----------------------------------------------------------------------*/

struct SAVED_ACTOR {
	short		actorID;
	short		zFactor;
	bool		bAlive;
	bool		bHidden;
	SCNHANDLE	presFilm;	///< the film that reel belongs to
	short		presRnum;	///< the present reel number
	short		presPlayX, presPlayY;
};
typedef SAVED_ACTOR *PSAVED_ACTOR;

#define NUM_ZPOSITIONS	200	// Reasonable-sounding number

struct Z_POSITIONS {
	short	actor;
	short	column;
	int		z;
};

void RestoreActorProcess(int id, INT_CONTEXT *pic, bool savegameFlag);

int SaveActors(PSAVED_ACTOR sActorInfo);
void RestoreActors(int numActors, PSAVED_ACTOR sActorInfo);

void SaveZpositions(void *zpp);
void RestoreZpositions(void *zpp);

void SaveActorZ(byte *saveActorZ);
void RestoreActorZ(byte *saveActorZ);

/*----------------------------------------------------------------------*/

} // End of namespace Tinsel

#endif /* TINSEL_ACTOR_H */
