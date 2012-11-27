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
 * Handles things to do with actors, delegates much moving actor stuff.
 */

#include "tinsel/actors.h"
#include "tinsel/background.h"
#include "tinsel/events.h"
#include "tinsel/film.h"	// for FREEL
#include "tinsel/handle.h"
#include "tinsel/dialogs.h"	// INV_NOICON
#include "tinsel/move.h"
#include "tinsel/multiobj.h"
#include "tinsel/object.h"	// for POBJECT
#include "tinsel/pcode.h"
#include "tinsel/pid.h"
#include "tinsel/play.h"
#include "tinsel/polygons.h"
#include "tinsel/rince.h"
#include "tinsel/sched.h"
#include "common/serializer.h"
#include "tinsel/sysvar.h"
#include "tinsel/tinsel.h"
#include "tinsel/token.h"

#include "common/textconsole.h"
#include "common/util.h"

namespace Tinsel {


//----------------- LOCAL DEFINES --------------------


#include "common/pack-start.h"	// START STRUCT PACKING

/** actor struct - one per actor */
struct T1_ACTOR_STRUC {
	int32 masking;			///< type of actor masking
	SCNHANDLE hActorId;		///< handle actor ID string index
	SCNHANDLE hActorCode;	///< handle to actor script
} PACKED_STRUCT;

struct T2_ACTOR_STRUC {
	SCNHANDLE hActorId;	// handle actor ID string index
	SCNHANDLE hTagText;	// tag
	int32 tagPortionV;	// defines tag area
	int32 tagPortionH;	// defines tag area
	SCNHANDLE hActorCode;	// handle to actor script
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

//----------------- LOCAL MACROS ----------------------------

#define RANGE_CHECK(num)	assert(num > 0 && num <= NumActors);

//----------------- LOCAL GLOBAL DATA --------------------

#define MAX_REELS 6

// FIXME: Avoid non-const global vars

static int LeadActorId = 0;		// The lead actor

static int NumActors = 0;	// The total number of actors in the game

struct ACTORINFO {
	bool		bAlive;		// TRUE == alive
	bool		bHidden;	// TRUE == hidden
	bool		completed;	// TRUE == script played out

	int			x, y, z;

	int32		mtype;		// DEFAULT(b'ground), MASK, ALWAYS
	SCNHANDLE	actorCode;	// The actor's script

	const FREEL	*presReel;	// the present reel
	int			presRnum;	// the present reel number
	SCNHANDLE	presFilm;	// the film that reel belongs to
	OBJECT		*presObj;	// reference for position information
	int			presPlayX, presPlayY;

	bool		tagged;		// actor tagged?
	SCNHANDLE	hTag;		// handle to tag text
	int			tType;		// e.g. TAG_Q1TO3

	bool		bEscOn;
	int			escEvent;

	COLORREF	textColor;	// Text color

	SCNHANDLE	playFilm;	// revert to this after talks
	SCNHANDLE	talkFilm;	// this be deleted in the future!
	SCNHANDLE	latestFilm;	// the last film ordered
	bool		bTalking;

	int			steps;
	int			loopCount;

	// DW2 new fields and alternates
	int			presColumns[MAX_REELS];	// the present columns
	OBJECT		*presObjs[MAX_REELS];	// reference for position information
	int			filmNum;
};

struct TAGACTOR {
	// Copies of compiled data
	int			id;
	SCNHANDLE	hTagText;		// handle to tag text
	int32		tagPortionV;	// which portion is active
	int32		tagPortionH;	// which portion is active
	SCNHANDLE	hActorCode;		// The actor's script

	int			tagFlags;
	SCNHANDLE	hOverrideTag;	// Override tag.
};
typedef TAGACTOR *PTAGACTOR;


static ACTORINFO *actorInfo = NULL;

static COLORREF defaultColor = 0;		// Text color

static bool bActorsOn = false;

static int ti = 0;

#define MAX_TAGACTORS 10

static TAGACTOR taggedActors[MAX_TAGACTORS];

static int numTaggedActors = 0;

static uint8 *zFactors = NULL;

static Z_POSITIONS zPositions[NUM_ZPOSITIONS];

//-------------------- METHOD LIST -----------------------

/**
 * Called once at start-up time, and again at restart time.
 * Registers the total number of actors in the game.
 * @param num			Chunk Id
 */
void RegisterActors(int num) {
	if (actorInfo == NULL)	{
		// Store the total number of actors in the game
		NumActors = num;

		// Check we can save so many
		assert(NumActors <= MAX_SAVED_ALIVES);

		// Allocate RAM for actor structures

		// FIXME: For now, we always allocate MAX_SAVED_ALIVES blocks,
		//   as this makes the save/load code simpler
		// size of ACTORINFO is 148, so this allocates 512 * 148 = 75776 bytes, about 74KB
		actorInfo = (ACTORINFO *)calloc(MAX_SAVED_ALIVES, sizeof(ACTORINFO));
		if (TinselV2)
			zFactors = (uint8 *)malloc(MAX_SAVED_ALIVES);

		// make sure memory allocated
		if (actorInfo == NULL) {
			error("Cannot allocate memory for actors");
		}
	} else {
		// Check the total number of actors is still the same
		assert(num == NumActors);

		memset(actorInfo, 0, MAX_SAVED_ALIVES * sizeof(ACTORINFO));
		if (TinselV2)
			memset(zFactors, 0, MAX_SAVED_ALIVES);
	}

	// All actors start off alive.
	while (num--)
		actorInfo[num].bAlive = true;
}

void FreeActors() {
	free(actorInfo);
	actorInfo = NULL;
	if (TinselV2) {
		free(zFactors);
		zFactors = NULL;
	}
}

/**
 * Called from dec_lead(), i.e. normally once at start of master script.
 * @param leadID			Lead Id
 */
void SetLeadId(int leadID) {
	LeadActorId = leadID;
	actorInfo[leadID-1].mtype = ACT_MASK;
}

/**
 * No comment.
 */
int GetLeadId() {
	return LeadActorId;
}

bool ActorIsGhost(int actor) {
	return actor == SysVar(ISV_GHOST_ACTOR);
}

struct ATP_INIT {
	int		id;		// Actor number
	TINSEL_EVENT	event;		// Event
	PLR_EVENT	bev;		// Causal mouse event

	PINT_CONTEXT	pic;
};

/**
 * Convert actor id to index into TaggedActors[]
 */
static int TaggedActorIndex(int actor) {
	int i;

	for (i = 0; i < numTaggedActors; i++) {
		if (taggedActors[i].id == actor)
			return i;
	}

	error("You may say to yourself \"this is not my tagged actor\"");
}

/**
 * Runs actor's glitter code.
 */
static void ActorTinselProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		INT_CONTEXT *pic;
		bool bTookControl;
	CORO_END_CONTEXT(_ctx);

	// get the stuff copied to process when it was created
	const ATP_INIT *atp = (const ATP_INIT *)param;

	CORO_BEGIN_CODE(_ctx);

	if (TinselV2) {
		// Take control for CONVERSE events
		if (atp->event == CONVERSE) {
			_ctx->bTookControl = GetControl();
			HideConversation(true);
		} else
			_ctx->bTookControl = false;

		// Run the Glitter code
		CORO_INVOKE_1(Interpret, atp->pic);

		// Restore conv window if applicable
		if (atp->event == CONVERSE) {
			// Free control if we took it
			if (_ctx->bTookControl)
				ControlOn();

			HideConversation(false);
		}
	} else {
		CORO_INVOKE_1(AllowDclick, atp->bev);		// May kill us if single click

		// Run the Glitter code
		assert(actorInfo[atp->id - 1].actorCode); // no code to run

		_ctx->pic = InitInterpretContext(GS_ACTOR, actorInfo[atp->id - 1].actorCode,
			atp->event, NOPOLY, atp->id, NULL);
		CORO_INVOKE_1(Interpret, _ctx->pic);

		// If it gets here, actor's code has run to completion
		actorInfo[atp->id - 1].completed = true;
	}

	CORO_END_CODE;
}


//---------------------------------------------------------------------------

struct RATP_INIT {
	INT_CONTEXT *pic;
	int		id;		// Actor number
};

static void ActorRestoredProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		INT_CONTEXT *pic;
	CORO_END_CONTEXT(_ctx);

	// get the stuff copied to process when it was created
	const RATP_INIT *r = (const RATP_INIT *)param;
	bool isSavegame = r->pic->resumeState == RES_SAVEGAME;

	CORO_BEGIN_CODE(_ctx);

	_ctx->pic = RestoreInterpretContext(r->pic);

	// The newly added check here specially sets the process to RES_NOT when loading a savegame.
	// This is needed particularly for the Psychiatrist scene in Discworld 1 - otherwise Rincewind
	// can't go upstairs without leaving the building and returning.  If this patch causes problems
	// in other scenes, an added check for the hCode == 1174490602 could be added.
	if (isSavegame && TinselV1)
		_ctx->pic->resumeState = RES_NOT;

	CORO_INVOKE_1(Interpret, _ctx->pic);

	// If it gets here, actor's code has run to completion
	actorInfo[r->id - 1].completed = true;

	CORO_END_CODE;
}

void RestoreActorProcess(int id, INT_CONTEXT *pic, bool savegameFlag) {
	RATP_INIT r = { pic, id };
	if (savegameFlag)
		pic->resumeState = RES_SAVEGAME;

	CoroScheduler.createProcess(PID_TCODE, ActorRestoredProcess, &r, sizeof(r));
}

/**
 * Starts up process to runs actor's glitter code.
 * @param ano			Actor Id
 * @param event			Event structure
 * @param be			ButEvent
 */
void ActorEvent(int ano, TINSEL_EVENT event, PLR_EVENT be) {
	ATP_INIT atp;

	// Only if there is Glitter code associated with this actor.
	if (actorInfo[ano - 1].actorCode) {
		atp.id = ano;
		atp.event = event;
		atp.bev = be;
		atp.pic = NULL;
		CoroScheduler.createProcess(PID_TCODE, ActorTinselProcess, &atp, sizeof(atp));
	}
}

/**
 * Starts up process to run actor's glitter code.
 */
void ActorEvent(CORO_PARAM, int ano, TINSEL_EVENT tEvent, bool bWait, int myEscape, bool *result) {
	ATP_INIT atp;
	int	index;
	CORO_BEGIN_CONTEXT;
		Common::PPROCESS pProc;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	index = TaggedActorIndex(ano);
	assert(taggedActors[index].hActorCode);
	if (result) *result = false;

	atp.id = 0;
	atp.event = tEvent;
	atp.pic = InitInterpretContext(GS_ACTOR,
			taggedActors[index].hActorCode,
			tEvent,
			NOPOLY,			// No polygon
			ano,			// Actor
			NULL,			// No object
			myEscape);

	if (atp.pic != NULL) {
		_ctx->pProc = CoroScheduler.createProcess(PID_TCODE, ActorTinselProcess, &atp, sizeof(atp));
		AttachInterpret(atp.pic, _ctx->pProc);

		if (bWait)
			CORO_INVOKE_2(WaitInterpret,_ctx->pProc, result);
	}

	CORO_END_CODE;
}


/**
 * Called at the start of each scene for each actor with a code block.
 * @param as			Actor structure
 * @param bRunScript	Flag for whether to run actor's script for the scene
 */
void StartActor(const T1_ACTOR_STRUC *as, bool bRunScript) {
	SCNHANDLE hActorId = FROM_LE_32(as->hActorId);

	// Zero-out many things
	actorInfo[hActorId - 1].bHidden = false;
	actorInfo[hActorId - 1].completed = false;
	actorInfo[hActorId - 1].x = 0;
	actorInfo[hActorId - 1].y = 0;
	actorInfo[hActorId - 1].presReel = NULL;
	actorInfo[hActorId - 1].presFilm = 0;
	actorInfo[hActorId - 1].presObj = NULL;

	// Store current scene's parameters for this actor
	actorInfo[hActorId - 1].mtype = FROM_LE_32(as->masking);
	actorInfo[hActorId - 1].actorCode = FROM_LE_32(as->hActorCode);

	// Run actor's script for this scene
	if (bRunScript) {
		if (bActorsOn)
			actorInfo[hActorId - 1].bAlive = true;

		if (actorInfo[hActorId - 1].bAlive && FROM_LE_32(as->hActorCode))
			ActorEvent(hActorId, STARTUP, PLR_NOEVENT);
	}
}

/**
 * Called at the start of each scene. Start each actor with a code block.
 * @param ah			Scene handle
 * @param numActors		Number of actors
 * @param bRunScript	Flag for whether to run actor scene scripts
 */
void StartTaggedActors(SCNHANDLE ah, int numActors, bool bRunScript) {
	int	i;

	if (TinselV2) {
		// Clear it all out for a fresh start
		memset(taggedActors, 0, sizeof(taggedActors));
		numTaggedActors = numActors;
	} else {
		// Only actors with code blocks got (x, y) re-initialized, so...
		for (i = 0; i < NumActors; i++) {
			actorInfo[i].x = actorInfo[i].y = 0;
			actorInfo[i].mtype = 0;
		}
	}

	if (!TinselV2) {
		// Tinsel 1 load variation
		const T1_ACTOR_STRUC *as = (const T1_ACTOR_STRUC *)LockMem(ah);
		for (i = 0; i < numActors; i++, as++) {
			StartActor(as, bRunScript);
		}
	} else if (numActors > 0) {
		// Tinsel 2 load variation
		const T2_ACTOR_STRUC *as = (T2_ACTOR_STRUC *)LockMem(ah);
		for (i = 0; i < numActors; i++, as++) {
			assert(as->hActorCode);

			// Store current scene's parameters for this tagged actor
			taggedActors[i].id			= FROM_LE_32(as->hActorId);
			taggedActors[i].hTagText	= FROM_LE_32(as->hTagText);
			taggedActors[i].tagPortionV	= FROM_LE_32(as->tagPortionV);
			taggedActors[i].tagPortionH	= FROM_LE_32(as->tagPortionH);
			taggedActors[i].hActorCode	= FROM_LE_32(as->hActorCode);

			// Run actor's script for this scene
			if (bRunScript) {
				// Send in reverse order - they get swapped round in the scheduler
				ActorEvent(Common::nullContext, taggedActors[i].id, SHOWEVENT, false, 0);
				ActorEvent(Common::nullContext, taggedActors[i].id, STARTUP, false, 0);
			}
		}
	}
}

/**
 * Called between scenes, zeroises all actors.
 */
void DropActors() {

	for (int i = 0; i < NumActors; i++) {
		if (TinselV2) {
			// Save text color
			COLORREF tColor = actorInfo[i].textColor;

			memset(&actorInfo[i], 0, sizeof(ACTORINFO));

			// Restor text color
			actorInfo[i].textColor = tColor;

			// Clear extra arrays
			memset(zFactors, 0, NumActors);
			memset(zPositions, 0, sizeof(zPositions));
		} else {
			// In Tinsel v1, only certain fields get reset
			actorInfo[i].actorCode = 0;		// No script
			actorInfo[i].presReel = NULL;	// No reel running
			actorInfo[i].presFilm = 0;		//   ditto
			actorInfo[i].presObj = NULL;	// No object
			actorInfo[i].x = 0;				// No position
			actorInfo[i].y = 0;				//   ditto

			actorInfo[i].talkFilm = 0;
			actorInfo[i].latestFilm = 0;
			actorInfo[i].playFilm = 0;
			actorInfo[i].bTalking = false;
		}
	}
}

/**
 * Kill actors.
 * @param ano			Actor Id
 */
void DisableActor(int ano) {
	PMOVER	pActor;

	assert(ano > 0 && ano <= NumActors); // illegal actor number

	actorInfo[ano - 1].bAlive = false;	// Record as dead
	actorInfo[ano - 1].x = actorInfo[ano - 1].y = 0;

	// Kill off moving actor properly
	pActor = GetMover(ano);
	if (pActor)
		KillMover(pActor);
}

/**
 * Enable actors.
 * @param ano			Actor Id
 */
void EnableActor(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	// Re-incarnate only if it's dead, or it's script ran to completion
	if (!actorInfo[ano - 1].bAlive || actorInfo[ano - 1].completed) {
		actorInfo[ano - 1].bAlive = true;
		actorInfo[ano - 1].bHidden = false;
		actorInfo[ano - 1].completed = false;

		// Re-run actor's script for this scene
		if (actorInfo[ano-1].actorCode)
			ActorEvent(ano, STARTUP, PLR_NOEVENT);
	}
}

/**
 * Returns the aliveness (to coin a word) of the actor.
 * @param ano			Actor Id
 */
bool actorAlive(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].bAlive;
}

/**
 * Define an actor as being tagged.
 * @param ano			Actor Id
 * @param tagtext		Scene handle
 * @param tp			tType
 */
void Tag_Actor(int ano, SCNHANDLE tagtext, int tp) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	actorInfo[ano-1].tagged = true;
	actorInfo[ano-1].hTag = tagtext;
	actorInfo[ano-1].tType = tp;
}

/**
 * Undefine  an actor as being tagged.
 * @param ano			Actor Id
 * @param tagtext		Scene handle
 * @param tp			tType
 */
void UnTagActor(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	actorInfo[ano-1].tagged = false;
}

/**
 * Redefine an actor as being tagged.
 * @param ano			Actor Id
 * @param tagtext		Scene handle
 * @param tp			tType
 */
void ReTagActor(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	if (actorInfo[ano-1].hTag)
		actorInfo[ano-1].tagged = true;
}

/**
 * Returns a tagged actor's tag type. e.g. TAG_Q1TO3
 * @param ano			Actor Id
 */
int TagType(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano-1].tType;
}

/**
 * Returns handle to tagged actor's tag text
 * @param ano			Actor Id
 */
SCNHANDLE GetActorTag(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].hTag;
}

/**
 * Called from TagProcess, FirstTaggedActor() resets the index, then
 * NextTagged Actor is repeatedly called until the caller gets fed up
 * or there are no more tagged actors to look at.
 */
void FirstTaggedActor() {
	ti = 0;
}

/**
 * Called from TagProcess, FirstTaggedActor() resets the index, then
 * NextTagged Actor is repeatedly called until the caller gets fed up
 * or there are no more tagged actors to look at.
 */
int NextTaggedActor() {
	PMOVER	pActor;
	bool	hid;

	while (ti < NumActors) {
		if (actorInfo[ti].tagged) {
			pActor = GetMover(ti+1);
			if (pActor)
				hid = MoverHidden(pActor);
			else
				hid = actorInfo[ti].bHidden;

			if (!hid) {
				return ++ti;
			}
		}
		++ti;
	}

	return 0;
}

/**
 * Called from TagProcess, NextTaggedActor() is
 * called repeatedly until the caller gets fed up or
 * there are no more tagged actors to look at.
 */
int NextTaggedActor(int previous) {
	PMOVER  pMover;

	// Convert actor number to index
	if (!previous)
		previous = -1;
	else
		previous = TaggedActorIndex(previous);

	while (++previous < numTaggedActors) {
		pMover = GetMover(taggedActors[previous].id);

		// No tag on lead actor while he's moving
		if ((taggedActors[previous].id) == GetLeadId() && MoverMoving(pMover)) {
			taggedActors[previous].tagFlags &= ~(POINTING | TAGWANTED);
			continue;
		}

		// Not if the actor doesn't exist at the moment
		if (pMover && !MoverIs(pMover))
			continue;

		if (!(pMover ? MoverHidden(pMover) : ActorHidden(taggedActors[previous].id))) {
			return taggedActors[previous].id;
		}
	}

	return 0;
}

/**
 * Returns the masking type of the actor.
 * @param ano			Actor Id
 */
int32 actorMaskType(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].mtype;
}

/**
 * Store/Return the currently stored co-ordinates of the actor.
 * Delegate the task for moving actors.
 * @param ano			Actor Id
 * @param x				X position
 * @param y				Y position
 */
void StoreActorPos(int ano, int x, int y) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	actorInfo[ano - 1].x = x;
	actorInfo[ano - 1].y = y;
}

void StoreActorSteps(int ano, int steps) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	actorInfo[ano - 1].steps = steps;
}

int GetActorSteps(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].steps;
}

void StoreActorZpos(int ano, int z, int column) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	if (!TinselV2) {
		// Prior to Tinsel 2, only a single z value was stored
		actorInfo[ano - 1].z = z;
	} else {
		// Alter existing entry, if there is one
		for (int i = 0; i < NUM_ZPOSITIONS; i++) {
			if (zPositions[i].actor == ano && zPositions[i].column == column) {
				zPositions[i].z = z;
				return;
			}
		}

		// No existing entry found, so find an empty slot
		for (int i = 0; i < NUM_ZPOSITIONS; i++) {
			if (zPositions[i].actor == 0) {
				zPositions[i].actor = (short)ano;
				zPositions[i].column = (short)column;
				zPositions[i].z = z;
				return;
			}
		}

		error("NUM_ZPOSITIONS exceeded");
	}
}

int GetActorZpos(int ano, int column) {
	RANGE_CHECK(ano);

	// Find entry, there should be one
	for (int i = 0; i < NUM_ZPOSITIONS; i++) {
		if (zPositions[i].actor == ano && zPositions[i].column == column) {
			return zPositions[i].z;
		}
	}

	return 1000;	// Nominal value
}

void IncLoopCount(int ano) {
	RANGE_CHECK(ano);

	actorInfo[ano - 1].loopCount++;
}

int GetLoopCount(int ano) {
	RANGE_CHECK(ano);

	return actorInfo[ano - 1].loopCount;
}

void GetActorPos(int ano, int *x, int *y) {
	PMOVER pActor;

	assert((ano > 0 && ano <= NumActors) || ano == LEAD_ACTOR); // unknown actor

	pActor = GetMover(ano);

	if (pActor)
		GetMoverPosition(pActor, x, y);
	else {
		*x = actorInfo[ano - 1].x;
		*y = actorInfo[ano - 1].y;
	}
}

/**
 * Returns the position of the mid-top of the actor.
 * Delegate the task for moving actors.
 * @param ano			Actor Id
 * @param x				Output x
 * @param y				Output y
 */
void GetActorMidTop(int ano, int *x, int *y) {
	// Not used in JAPAN version
	PMOVER pActor;

	assert((ano > 0 && ano <= NumActors) || ano == LEAD_ACTOR); // unknown actor

	pActor = GetMover(ano);

	if (pActor)
		GetMoverMidTop(pActor, x, y);
	else if (TinselV2) {
		*x = (GetActorLeft(ano) + GetActorRight(ano)) / 2;
		*y = GetActorTop(ano);
	} else if (actorInfo[ano - 1].presObj) {
		*x = (MultiLeftmost(actorInfo[ano - 1].presObj)
		      + MultiRightmost(actorInfo[ano - 1].presObj)) / 2;
		*y = MultiHighest(actorInfo[ano - 1].presObj);
	} else
		GetActorPos(ano, x, y);		// The best we can do!
}

/**
 * Return the appropriate co-ordinate of the actor.
 * @param ano			Actor Id
 */
int GetActorLeft(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	if (!TinselV2) {
		// Tinsel 1 version
		if (!actorInfo[ano - 1].presObj)
			return 0;

		return MultiLeftmost(actorInfo[ano - 1].presObj);
	}

	// Tinsel 2 version
	PMOVER pMover = GetMover(ano);
	int i;
	bool bIsObj;
	int left = 0;

	if (pMover != NULL) {
		return GetMoverLeft(pMover);
	} else {
		for (i = 0, bIsObj = false; i < MAX_REELS; i++) {
			// If there's an object
			// and it is not a blank frame for it...
			if (actorInfo[ano-1].presObjs[i] && MultiHasShape(actorInfo[ano - 1].presObjs[i])) {
				if (!bIsObj) {
					bIsObj = true;
					left = MultiLeftmost(actorInfo[ano - 1].presObjs[i]);
				} else {
					if (MultiLeftmost(actorInfo[ano - 1].presObjs[i]) < left)
						left = MultiLeftmost(actorInfo[ano - 1].presObjs[i]);
				}
			}
		}

		return bIsObj ? left : 0;
	}
}

/**
 * Return the appropriate co-ordinate of the actor.
 * @param ano			Actor Id
 */
int GetActorRight(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	if (!TinselV2) {
		// Tinsel 1 version
		if (!actorInfo[ano - 1].presObj)
			return 0;

		return MultiRightmost(actorInfo[ano - 1].presObj);
	}

	// Tinsel 2 version
	PMOVER pMover = GetMover(ano);
	int i;
	bool bIsObj;
	int right = 0;

	if (pMover != NULL) {
		return GetMoverRight(pMover);
	} else {
		for (i = 0, bIsObj = false; i < MAX_REELS; i++) {
			// If there's an object
			// and it is not a blank frame for it...
			if (actorInfo[ano-1].presObjs[i] && MultiHasShape(actorInfo[ano-1].presObjs[i])) {
				if (!bIsObj) {
					bIsObj = true;
					right = MultiRightmost(actorInfo[ano-1].presObjs[i]);
				} else {
					if (MultiRightmost(actorInfo[ano-1].presObjs[i]) > right)
						right = MultiRightmost(actorInfo[ano-1].presObjs[i]);
				}
			}
		}
		return bIsObj ? right : 0;
	}
}

/**
 * Return the appropriate co-ordinate of the actor.
 * @param ano			Actor Id
 */
int GetActorTop(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	if (!TinselV2) {
		// Tinsel 1 version
		if (!actorInfo[ano - 1].presObj)
			return 0;

		return MultiHighest(actorInfo[ano - 1].presObj);
	}

	// Tinsel 2 version
	PMOVER pMover = GetMover(ano);
	int i;
	bool bIsObj;
	int top = 0;

	if (pMover != NULL) {
		return GetMoverTop(pMover);
	} else {
		for (i = 0, bIsObj = false; i < MAX_REELS; i++) {
			// If there's an object
			// and it is not a blank frame for it...
			if (actorInfo[ano-1].presObjs[i] && MultiHasShape(actorInfo[ano-1].presObjs[i])) {
				if (!bIsObj) {
					bIsObj = true;
					top = MultiHighest(actorInfo[ano-1].presObjs[i]);
				} else {
					if (MultiHighest(actorInfo[ano-1].presObjs[i]) < top)
						top = MultiHighest(actorInfo[ano-1].presObjs[i]);
				}
			}
		}

		return bIsObj ? top : 0;
	}
}

/**
 * Return the appropriate co-ordinate of the actor.
 */
int GetActorBottom(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	if (!TinselV2) {
		// Tinsel 1 version
		if (!actorInfo[ano - 1].presObj)
			return 0;

		return MultiLowest(actorInfo[ano - 1].presObj);
	}

	// Tinsel 2 version
	PMOVER pMover = GetMover(ano);
	int i;
	bool bIsObj;
	int bottom = 0;

	if (pMover != NULL) {
		return GetMoverBottom(pMover);
	} else {
		for (i = 0, bIsObj = false; i < MAX_REELS; i++) {
			// If there's an object
			// and it is not a blank frame for it...
			if (actorInfo[ano-1].presObjs[i] && MultiHasShape(actorInfo[ano-1].presObjs[i])) {
				if (!bIsObj) {
					bIsObj = true;
					bottom = MultiLowest(actorInfo[ano-1].presObjs[i]);
				} else {
					if (MultiLowest(actorInfo[ano-1].presObjs[i]) > bottom)
						bottom = MultiLowest(actorInfo[ano-1].presObjs[i]);
				}
			}
		}
		return bIsObj ? bottom : 0;
	}
}

/**
 * Shows the given actor
 */
void ShowActor(CORO_PARAM, int ano) {
	PMOVER pMover;
	RANGE_CHECK(ano);

	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// reset hidden flag
	actorInfo[ano - 1].bHidden = false;

	// Send event to tagged actors
	if (IsTaggedActor(ano))
		CORO_INVOKE_ARGS(ActorEvent, (CORO_SUBCTX, ano, SHOWEVENT, true, 0));

	// If moving actor involved, un-hide it
	pMover = GetMover(ano);
	if (pMover)
		UnHideMover(pMover);

	CORO_END_CODE;
}

/**
 * Set actor hidden status to true.
 * For a moving actor, actually hide it.
 * @param ano			Actor Id
 */
void HideActor(CORO_PARAM, int ano) {
	PMOVER pMover;
	assert((ano > 0 && ano <= NumActors) || ano == LEAD_ACTOR); // illegal actor

	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (TinselV2) {
		actorInfo[ano - 1].bHidden = true;

		// Send event to tagged actors
		// (this is duplicated in HideMover())
		if (IsTaggedActor(ano)) {
			CORO_INVOKE_ARGS(ActorEvent, (CORO_SUBCTX, ano, HIDEEVENT, true, 0));

			// It may be pointed to
			SetActorPointedTo(ano, false);
			SetActorTagWanted(ano, false, false, 0);
		}
	}

	// Get moving actor involved
	pMover = GetMover(ano);

	if (pMover)
		HideMover(pMover, 0);
	else if (!TinselV2)
		actorInfo[ano - 1].bHidden = true;

	CORO_END_CODE;
}

/**
 * Return actor hidden status.
 */
bool ActorHidden(int ano) {
	RANGE_CHECK(ano);

	return actorInfo[ano - 1].bHidden;
}

/**
 * Hide an actor if it's a moving actor.
 * @param ano			Actor Id
 * @param sf			sf
 */
bool HideMovingActor(int ano, int sf) {
	PMOVER pActor;

	assert((ano > 0 && ano <= NumActors) || ano == LEAD_ACTOR); // illegal actor

	// Get moving actor involved
	pActor = GetMover(ano);

	if (pActor) {
		HideMover(pActor, sf);
		return true;
	} else {
		if (actorInfo[ano - 1].presObj != NULL)
			MultiHideObject(actorInfo[ano - 1].presObj);	// Hidee object
		return false;
	}
}

/**
 * Unhide an actor if it's a moving actor.
 * @param ano			Actor Id
 */
void unHideMovingActor(int ano) {
	PMOVER pActor;

	assert((ano > 0 && ano <= NumActors) || ano == LEAD_ACTOR); // illegal actor

	// Get moving actor involved
	pActor = GetMover(ano);

	assert(pActor); // not a moving actor

	UnHideMover(pActor);
}

/**
 * Called after a moving actor had been replaced by an splay().
 * Moves the actor to where the splay() left it, and continues the
 * actor's walk (if any) from the new co-ordinates.
 */
void restoreMovement(int ano) {
	PMOVER pActor;

	assert(ano > 0 && ano <= NumActors); // illegal actor number

	// Get moving actor involved
	pActor = GetMover(ano);

	assert(pActor); // not a moving actor

	if (pActor->objX == actorInfo[ano - 1].x && pActor->objY == actorInfo[ano - 1].y)
		return;

	pActor->objX = actorInfo[ano - 1].x;
	pActor->objY = actorInfo[ano - 1].y;

	if (pActor->actorObj)
		SSetActorDest(pActor);
}

/**
 * More properly should be called:
 * 'store_actor_reel_and/or_film_and/or_object()'
 */
void storeActorReel(int ano, const FREEL *reel, SCNHANDLE hFilm, OBJECT *pobj, int reelnum, int x, int y) {
	PMOVER pActor;

	assert(ano > 0 && ano <= NumActors); // illegal actor number

	pActor = GetMover(ano);

	// Only store the reel and film for a moving actor if NOT called from MoverProcess()
	// (MoverProcess() calls with reel=film=NULL, pobj not NULL)
	if (!pActor
	|| !(reel == NULL && hFilm == 0 && pobj != NULL)) {
		actorInfo[ano - 1].presReel = reel;	// Store reel
		actorInfo[ano - 1].presRnum = reelnum;	// Store reel number
		actorInfo[ano - 1].presFilm = hFilm;	// Store film
		actorInfo[ano - 1].presPlayX = x;
		actorInfo[ano - 1].presPlayY = y;
	}

	// Only store the object for a moving actor if called from MoverProcess()
	if (!pActor) {
		actorInfo[ano - 1].presObj = pobj;	// Store object
	} else if (reel == NULL && hFilm == 0 && pobj != NULL) {
		actorInfo[ano - 1].presObj = pobj;	// Store object
	}
}

/**
 * Return the present reel/film of the actor.
 */
const FREEL *actorReel(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].presReel;	// the present reel
}

/***************************************************************************/

void SetActorPlayFilm(int ano, SCNHANDLE hFilm) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	actorInfo[ano - 1].playFilm = hFilm;
}

SCNHANDLE GetActorPlayFilm(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].playFilm;
}

void SetActorTalkFilm(int ano, SCNHANDLE hFilm) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	actorInfo[ano - 1].talkFilm = hFilm;
}

SCNHANDLE GetActorTalkFilm(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].talkFilm;
}

void SetActorTalking(int ano, bool tf) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	actorInfo[ano - 1].bTalking = tf;
}

bool ActorIsTalking(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].bTalking;
}

void SetActorLatestFilm(int ano, SCNHANDLE hFilm) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	actorInfo[ano - 1].latestFilm = hFilm;
	actorInfo[ano - 1].steps = 0;
}

SCNHANDLE GetActorLatestFilm(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].latestFilm;
}

/***************************************************************************/

void UpdateActorEsc(int ano, bool escOn, int escEvent) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	actorInfo[ano - 1].bEscOn = escOn;
	actorInfo[ano - 1].escEvent = escEvent;
}

void UpdateActorEsc(int ano, int escEvent) {
	RANGE_CHECK(ano);

	if (escEvent) {
		actorInfo[ano - 1].bEscOn = true;
		actorInfo[ano - 1].escEvent = escEvent;
	} else {
		actorInfo[ano - 1].bEscOn = false;
		actorInfo[ano - 1].escEvent = GetEscEvents();
	}

}

bool ActorEsc(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].bEscOn;
}

int ActorEev(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].escEvent;
}

/**
 * Guess what these do.
 */
int AsetZPos(OBJECT *pObj, int y, int32 z) {
	int zPos;

	z += z ? -1 : 0;

	zPos = y + (z << ZSHIFT);
	MultiSetZPosition(pObj, zPos);
	return zPos;
}

/**
 * Guess what these do.
 */
void SetMoverZ(PMOVER pMover, int y, int32 zFactor) {
	if (!pMover->bHidden) {
		if (!TinselV2)
			AsetZPos(pMover->actorObj, y, zFactor);
		else if (MoverIsSWalking(pMover) && pMover->zOverride != -1) {
			// Special for SWalk()
			MultiSetZPosition(pMover->actorObj, (pMover->zOverride << ZSHIFT) + y);
		} else {
			// Normal case
			MultiSetZPosition(pMover->actorObj, (zFactor << ZSHIFT) + y);
		}
	}
}

/**
 * Stores actor's attributes.
 * Currently only the speech colors.
 */
void storeActorAttr(int ano, int r1, int g1, int b1) {
	assert((ano > 0 && ano <= NumActors) || ano == -1); // illegal actor number

	if (r1 > MAX_INTENSITY)	r1 = MAX_INTENSITY;	// } Ensure
	if (g1 > MAX_INTENSITY)	g1 = MAX_INTENSITY;	// } within limits
	if (b1 > MAX_INTENSITY)	b1 = MAX_INTENSITY;	// }

	if (ano == -1)
		defaultColor = TINSEL_RGB(r1, g1, b1);
	else
		actorInfo[ano - 1].textColor = TINSEL_RGB(r1, g1, b1);
}

/**
 * Called from ActorRGB() - Stores actor's speech color.
 */

void SetActorRGB(int ano, COLORREF color) {
	assert(ano >= 0 && ano <= NumActors);

	if (ano)
		actorInfo[ano - 1].textColor = TO_LE_32(color);
	else
		defaultColor = TO_LE_32(color);
}

/**
 * Get the actor's stored speech color.
 * @param ano			Actor Id
 */
COLORREF GetActorRGB(int ano) {
	// Not used in JAPAN version
	assert((ano >= -1) && (ano <= NumActors)); // illegal actor number

	if ((ano == -1) || !actorInfo[ano - 1].textColor)
		return defaultColor;
	else
		return actorInfo[ano - 1].textColor;
}

/**
 * Set the actor's Z-factor
 */
void SetActorZfactor(int ano, uint32 zFactor) {
	RANGE_CHECK(ano);

	zFactors[ano - 1] = (uint8)zFactor;
}

uint32 GetActorZfactor(int ano) {
	RANGE_CHECK(ano);

	return zFactors[ano - 1];
}

/**
 * Store relevant information pertaining to currently existing actors.
 */
int SaveActors(SAVED_ACTOR *sActorInfo) {
	int	i, j, k;

	for (i = 0, j = 0; i < NumActors; i++) {
		for (k = 0; k < (TinselV2 ? MAX_REELS : 1); ++k) {
			bool presFlag = !TinselV2 ? actorInfo[i].presObj != NULL :
				(actorInfo[i].presObjs[k] != NULL) && !IsCdPlayHandle(actorInfo[i].presFilm);
			if (presFlag) {

				assert(j < MAX_SAVED_ACTORS); // Saving too many actors

				if (!TinselV2) {
					sActorInfo[j].bAlive	= actorInfo[i].bAlive;
					sActorInfo[j].zFactor	= (short)actorInfo[i].z;
					sActorInfo[j].presRnum	= (short)actorInfo[i].presRnum;
				}

				sActorInfo[j].actorID	= (short)(i+1);
				if (TinselV2)
					sActorInfo[j].bHidden	= actorInfo[i].bHidden;
	//			sActorInfo[j].x		= (short)actorInfo[i].x;
	//			sActorInfo[j].y		= (short)actorInfo[i].y;
	//			sActorInfo[j].presReel	= actorInfo[i].presReel;
				sActorInfo[j].presFilm	= actorInfo[i].presFilm;
				sActorInfo[j].presPlayX	= (short)actorInfo[i].presPlayX;
				sActorInfo[j].presPlayY	= (short)actorInfo[i].presPlayY;
				j++;

				break;
			}
		}
	}

	return j;
}

/**
 * Restore actor data
 */
void RestoreActors(int numActors, PSAVED_ACTOR sActorInfo) {
	int	i, aIndex;

	for (i = 0; i < numActors; i++) {
		aIndex = sActorInfo[i].actorID - 1;

		actorInfo[aIndex].bHidden = sActorInfo[i].bHidden;

		// Play the same reel.
		if (sActorInfo[i].presFilm != 0) {
			RestoreActorReels(sActorInfo[i].presFilm, sActorInfo[i].actorID,
				sActorInfo[i].presPlayX, sActorInfo[i].presPlayY);
		}
	}
}

void SaveZpositions(void *zpp) {
	memcpy(zpp, zPositions, sizeof(zPositions));
}

void RestoreZpositions(void *zpp) {
	memcpy(zPositions, zpp, sizeof(zPositions));
}

void SaveActorZ(byte *saveActorZ) {
	assert(NumActors <= MAX_SAVED_ACTOR_Z);

	memcpy(saveActorZ, zFactors, NumActors);
}

void RestoreActorZ(byte *saveActorZ) {
	memcpy(zFactors, saveActorZ, NumActors);
}

void setactorson() {
	bActorsOn = true;
}

void ActorsLife(int ano, bool bAlive) {
	assert((ano > 0 && ano <= NumActors) || ano == -1); // illegal actor number

	actorInfo[ano-1].bAlive = bAlive;
}


void syncAllActorsAlive(Common::Serializer &s) {
	for (int i = 0; i < MAX_SAVED_ALIVES; i++) {
		s.syncAsByte(actorInfo[i].bAlive);
		s.syncAsByte(actorInfo[i].tagged);
		s.syncAsByte(actorInfo[i].tType);
		s.syncAsUint32LE(actorInfo[i].hTag);
	}
}

/**
 * Called from EndActor()
 */
void dwEndActor(int ano) {
	int i;

	RANGE_CHECK(ano);

	// Make play.c think it's been replaced
// The following line may have been indirectly making text go away!
//	actorInfo[ano - 1].presFilm = NULL;
// but things were returning after a cut scene.
// so re-instate it and de-register the object
	actorInfo[ano - 1].presFilm = 0;
	actorInfo[ano-1].filmNum++;

	for (i = 0; i < MAX_REELS; i++) {
		// It may take a frame to remove this, so make it invisible
		if (actorInfo[ano-1].presObjs[i] != NULL) {
			MultiHideObject(actorInfo[ano-1].presObjs[i]);
			actorInfo[ano-1].presObjs[i] = NULL;
		}
	}
}


/**
 * Returns a tagged actor's tag portion.
 */
void GetActorTagPortion(int ano, unsigned *top, unsigned *bottom, unsigned *left, unsigned *right) {
	// Convert actor number to index
	ano = TaggedActorIndex(ano);

	*top = taggedActors[ano].tagPortionV >> 16;
	*bottom = taggedActors[ano].tagPortionV & 0xffff;
	*left = taggedActors[ano].tagPortionH >> 16;
	*right = taggedActors[ano].tagPortionH & 0xffff;

	// ensure validity
	assert(*top >= 1 && *top <= 8);
	assert(*bottom >= *top && *bottom <= 8);
	assert(*left >= 1 && *left <= 8);
	assert(*right >= *left && *right <= 8);
}

/**
 * Returns handle to tagged actor's tag text.
 */
SCNHANDLE GetActorTagHandle(int ano) {
	// Convert actor number to index
	ano = TaggedActorIndex(ano);

	return taggedActors[ano].hOverrideTag ?
		taggedActors[ano].hOverrideTag : taggedActors[ano].hTagText;
}

void SetActorPointedTo(int actor, bool bPointedTo) {
	// Convert actor number to index
	actor = TaggedActorIndex(actor);

	if (bPointedTo)
		taggedActors[actor].tagFlags |= POINTING;
	else
		taggedActors[actor].tagFlags &= ~POINTING;
}

bool ActorIsPointedTo(int actor) {
	// Convert actor number to index
	actor = TaggedActorIndex(actor);

	return (taggedActors[actor].tagFlags & POINTING);
}

void SetActorTagWanted(int actor, bool bTagWanted, bool bCursor, SCNHANDLE hOverrideTag) {
	// Convert actor number to index
	actor = TaggedActorIndex(actor);

	if (bTagWanted) {
		taggedActors[actor].tagFlags |= TAGWANTED;
		taggedActors[actor].hOverrideTag = hOverrideTag;
	} else {
		taggedActors[actor].tagFlags &= ~TAGWANTED;
		taggedActors[actor].hOverrideTag = 0;
	}

	if (bCursor)
		taggedActors[actor].tagFlags |= FOLLOWCURSOR;
	else
		taggedActors[actor].tagFlags &= ~FOLLOWCURSOR;
}

bool ActorTagIsWanted(int actor) {
	// Convert actor number to index
	actor = TaggedActorIndex(actor);

	return (taggedActors[actor].tagFlags & TAGWANTED);
}

/**
 * Given cursor position and an actor number, ascertains
 * whether the cursor is within the actor's tag area.
 * Returns True for a positive result, False for negative.
 */
bool InHotSpot(int ano, int curX, int curY) {
	int	aTop, aBot;	// Top and bottom limits }
	int	aHeight;	// Height		 } of active area
	int	aLeft, aRight;	// Left and right	 }
	int	aWidth;		// Width		 }
	unsigned topEighth, botEighth, leftEighth, rightEighth;

	// First check if within broad range
	if (curX < (aLeft = GetActorLeft(ano))		// too far left
		||  curX > (aRight = GetActorRight(ano))	// too far right
		||  curY < (aTop = GetActorTop(ano))		// too high
		||  curY > (aBot = GetActorBottom(ano)) )	// too low
			return false;

	GetActorTagPortion(ano, &topEighth, &botEighth, &leftEighth, &rightEighth);

	aWidth = aRight - aLeft;
	aLeft += ((leftEighth - 1)*aWidth)/8;
	aRight -= ((8 - rightEighth)*aWidth)/8;

	// check if within x-range
	if (curX < aLeft || curX > aRight)
		return false;

	aHeight = aBot - aTop;
	aTop += ((topEighth - 1)*aHeight)/8;
	aBot -= ((8 - botEighth)*aHeight)/8;

	// check if within y-range
	if (curY < aTop || curY > aBot)
		return false;

	return true;
}

/**
 * Front Tagged Actor
 */
int FrontTaggedActor() {
	int i;

	for (i = 0; i < numTaggedActors; i++) {
		if (taggedActors[i].tagFlags & POINTING)
			return taggedActors[i].id;
	}
	return 0;
}

/**
 * GetActorTagPos
 */
void GetActorTagPos(int actor, int *pTagX, int *pTagY, bool bAbsolute) {
	unsigned topEighth, botEighth;
	int	aTop;		// Top and bottom limits }
	int	aHeight;	// Height		 } of active area
	int	Loffset, Toffset;

	GetActorTagPortion(actor, &topEighth, &botEighth, (unsigned *)&Loffset, (unsigned *)&Toffset);

	aTop = GetActorTop(actor);
	aHeight = GetActorBottom(actor) - aTop;
	aTop += ((topEighth - 1) * aHeight) / 8;

	*pTagX = ((GetActorLeft(actor) + GetActorRight(actor)) / 2);
	*pTagY = aTop;

	if (!bAbsolute) {
		PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);
		*pTagX -= Loffset;
		*pTagY -= Toffset;
	}
}

/**
 * Is Tagged Actor
 */
bool IsTaggedActor(int actor) {
	int i;

	for (i = 0; i < numTaggedActors; i++) {
		if (taggedActors[i].id == actor)
			return true;
	}
	return false;
}

/**
 * StoreActorPresFilm
 */
void StoreActorPresFilm(int ano, SCNHANDLE hFilm, int x, int y) {
	int i;

	RANGE_CHECK(ano);

	actorInfo[ano-1].presFilm = hFilm;
	actorInfo[ano-1].presPlayX = x;
	actorInfo[ano-1].presPlayY = y;
	actorInfo[ano-1].filmNum++;

	for (i = 0; i < MAX_REELS; i++) {
		// It may take a frame to remove this, so make it invisible
		if (actorInfo[ano - 1].presObjs[i] != NULL)
			MultiHideObject(actorInfo[ano - 1].presObjs[i]);

		actorInfo[ano - 1].presColumns[i] = -1;
		actorInfo[ano - 1].presObjs[i] = NULL;
	}
}

/**
 * GetActorPresFilm
 */
SCNHANDLE GetActorPresFilm(int ano) {
	RANGE_CHECK(ano);

	return actorInfo[ano - 1].presFilm;
}


/**
 * GetActorFilmNumber
 */
int GetActorFilmNumber(int ano) {
	RANGE_CHECK(ano);

	return actorInfo[ano - 1].filmNum;
}

/**
 * More properly should be called:
 *		'StoreActorReelAndObject()'
 */
void StoreActorReel(int actor, int column, OBJECT *pObj) {
	RANGE_CHECK(actor);
	int i;

	for (i = 0; i < MAX_REELS; i++) {
		if (actorInfo[actor-1].presColumns[i] == -1) {
			// Store reel and object
			actorInfo[actor - 1].presColumns[i] = column;
			actorInfo[actor - 1].presObjs[i] = pObj;
			break;
		}
	}

	assert(i < MAX_REELS);
}

/**
 * NotPlayingReel
 */
void NotPlayingReel(int actor, int filmNumber, int column) {
	int	i;

	RANGE_CHECK(actor);

	if (actorInfo[actor-1].filmNum != filmNumber)
		return;

	// De-register this reel
	for (i = 0; i < MAX_REELS; i++) {
		if (actorInfo[actor-1].presColumns[i] == column) {
			actorInfo[actor-1].presObjs[i] = NULL;
			actorInfo[actor-1].presColumns[i] = -1;
			break;
		}
	}

	// De-register the film if this was the last reel
	for (i = 0; i < MAX_REELS; i++) {
		if (actorInfo[actor-1].presColumns[i] != -1)
			break;
	}
	if (i == MAX_REELS)
		actorInfo[actor-1].presFilm = 0;
}

bool ActorReelPlaying(int actor, int column) {
	RANGE_CHECK(actor);

	for (int i = 0; i < MAX_REELS; i++) {
		if (actorInfo[actor - 1].presColumns[i] == column)
			return true;
	}
	return false;
}

} // End of namespace Tinsel
