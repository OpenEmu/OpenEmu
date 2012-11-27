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
 * Plays films within a scene, takes into account the actor in each 'column'.								|
 */

#include "common/coroutines.h"
#include "tinsel/actors.h"
#include "tinsel/background.h"
#include "tinsel/dw.h"
#include "tinsel/film.h"
#include "tinsel/handle.h"
#include "tinsel/multiobj.h"
#include "tinsel/object.h"
#include "tinsel/pid.h"
#include "tinsel/play.h"
#include "tinsel/polygons.h"
#include "tinsel/rince.h"
#include "tinsel/sched.h"
#include "tinsel/scn.h"
#include "tinsel/sound.h"
#include "tinsel/timers.h"
#include "tinsel/tinlib.h"	// Stand()

namespace Tinsel {

struct PPINIT {
	SCNHANDLE hFilm;	// The 'film'
	int16	x;			// } Co-ordinates from the play()
	int16	y;			// } - set to (-1, -1) if none.
	int16	z;			// normally 0, set if from restore
	int16	speed;		// Film speed
	int16	actorid;	// Set if called from an actor code block
	uint8	splay;		// Set if called from splay()
	uint8	bTop;		// Set if called from topplay()
	uint8	bRestore;
	int16	sf;			// SlowFactor - only used for moving actors
	int16	column;		// Column number, first column = 0

	uint8	escOn;
	int32	myescEvent;
};

//----------------- LOCAL GLOBAL DATA --------------------

// FIXME: Avoid non-const global vars

static SOUNDREELS g_soundReels[MAX_SOUNDREELS];
static int g_soundReelNumbers[MAX_SOUNDREELS];

static int g_soundReelWait;

//-------------------- METHODS ----------------------

/**
 * Poke the background palette into an image.
 */
static void PokeInPalette(SCNHANDLE hMulFrame) {
	const FRAME *pFrame;		// Pointer to frame
	IMAGE *pim;		// Pointer to image

	// Could be an empty column
	if (hMulFrame) {
		pFrame = (const FRAME *)LockMem(hMulFrame);

		// get pointer to image
		pim = (IMAGE *)LockMem(READ_LE_UINT32(pFrame));	// handle to image

		pim->hImgPal = TO_LE_32(BgPal());
	}
}

/**
 * Poke the background palette into an image.
 */
void PokeInPalette(const MULTI_INIT *pmi) {
	FRAME	*pFrame;		// Pointer to frame
	IMAGE	*pim;			// Pointer to image

	// Could be an empty column
	if (pmi->hMulFrame) {
		pFrame = (FRAME *)LockMem(FROM_LE_32(pmi->hMulFrame));

		// get pointer to image
		pim = (IMAGE *)LockMem(READ_LE_UINT32(pFrame));	// handle to image

		pim->hImgPal = TO_LE_32(BgPal());
	}
}

int32 NoNameFunc(int actorID, bool bNewMover) {
	PMOVER	pActor;
	int32	retval;

	pActor = GetMover(actorID);

	if (pActor != NULL && !bNewMover) {
		// If no path, just use first path in the scene
		if (pActor->hCpath == NOPOLY)
			retval = GetPolyZfactor(FirstPathPoly());
		else
			retval = GetPolyZfactor(pActor->hCpath);
	} else {
		switch (actorMaskType(actorID)) {
		case ACT_DEFAULT:
			retval = 0;
			break;
		case ACT_MASK:
			retval = 0;
			break;
		case ACT_ALWAYS:
			retval = 10;
			break;
		default:
			retval = actorMaskType(actorID);
			break;
		}
	}

	return retval;
}

static FREEL *GetReel(SCNHANDLE hFilm, int column) {
	FILM *pFilm = (FILM *)LockMem(hFilm);

	return &pFilm->reels[column];
}

static int RegisterSoundReel(SCNHANDLE hFilm, int column, int actorCol) {
	int i;

	for (i = 0; i < MAX_SOUNDREELS; i++) {
		// Should assert this doesn't happen, but let's be tolerant
		if (g_soundReels[i].hFilm == hFilm && g_soundReels[i].column == column)
			break;

		if (!g_soundReels[i].hFilm) {
			g_soundReels[i].hFilm = hFilm;
			g_soundReels[i].column = column;
			g_soundReels[i].actorCol = actorCol;
			break;
		}
	}

	g_soundReelNumbers[i]++;
	return i;
}

void NoSoundReels() {
	memset(g_soundReels, 0, sizeof(g_soundReels));
	g_soundReelWait = 0;
}

static void DeRegisterSoundReel(SCNHANDLE hFilm, int column) {
	for (int i = 0; i < MAX_SOUNDREELS; i++) {
		// Should assert this doesn't happen, but let's be tolerant
		if (g_soundReels[i].hFilm == hFilm && g_soundReels[i].column == column) {
			g_soundReels[i].hFilm = 0;
			break;
		}
	}
}

void SaveSoundReels(PSOUNDREELS psr) {
	for (int i = 0; i < MAX_SOUNDREELS; i++) {
		if (IsCdPlayHandle(g_soundReels[i].hFilm))
			g_soundReels[i].hFilm = 0;
	}

	memcpy(psr, g_soundReels, sizeof(g_soundReels));
}

void RestoreSoundReels(PSOUNDREELS psr) {
	memcpy(g_soundReels, psr, sizeof(g_soundReels));
}

static uint32 GetZfactor(int actorID, PMOVER pMover, bool bNewMover) {
	if (pMover != NULL && bNewMover == false) {
		// If no path, just use first path in the scene
		if (pMover->hCpath == NOPOLY)
			return GetPolyZfactor(FirstPathPoly());
		else
			return GetPolyZfactor(pMover->hCpath);
	} else {
		return GetActorZfactor(actorID);
	}
}

/**
 * Handles reels with sound id.
 * @param hFilm				The 'film'
 * @param column			Column number, first column = 0
 * @param speed				Film speed
 */
static void SoundReel(CORO_PARAM, SCNHANDLE hFilm, int column, int speed,
					int myescEvent, int actorCol) {
	FILM *pFilm;
	FREEL *pReel;
	ANI_SCRIPT *pAni;

	short x, y;

	CORO_BEGIN_CONTEXT;
		int myId;
		int myNum;
		int frameNumber;
		int speed;
		int sampleNumber;
		bool bFinished;
		bool bLooped;
		int reelActor;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (actorCol) {
		PMULTI_INIT pmi;		// MULTI_INIT structure

		pReel = GetReel(hFilm, actorCol - 1);
		pmi = (PMULTI_INIT) LockMem(FROM_LE_32(pReel->mobj));
		_ctx->reelActor = (int32)FROM_LE_32(pmi->mulID);
	} else
		_ctx->reelActor = 0;

	_ctx->frameNumber = 0;
	_ctx->speed = speed;
	_ctx->sampleNumber = 0;
	_ctx->bFinished = false;
	_ctx->bLooped = false;
	_ctx->myId = RegisterSoundReel(hFilm, column, actorCol);
	_ctx->myNum = g_soundReelNumbers[_ctx->myId];

	do {
		pFilm = (FILM *)LockMem(hFilm);
		pReel = &pFilm->reels[column];

		pAni = (ANI_SCRIPT *)LockMem(FROM_LE_32(pReel->script));

		if (_ctx->speed == -1) {
			_ctx->speed = (ONE_SECOND/FROM_LE_32(pFilm->frate));

			// Restored reel
			for (;;) {
				if (FROM_LE_32(pAni[_ctx->frameNumber].op) == ANI_END)
					break;
				else if (FROM_LE_32(pAni[_ctx->frameNumber].op) == ANI_JUMP) {
					_ctx->frameNumber++;
					_ctx->frameNumber += FROM_LE_32(pAni[_ctx->frameNumber].op);
					break;
				}
				// Could check for the other stuff here
				// but they really dont happen
				// OH YES THEY DO
				else if (FROM_LE_32(pAni[_ctx->frameNumber].op) == ANI_ADJUSTX
					||	 FROM_LE_32(pAni[_ctx->frameNumber].op) == ANI_ADJUSTY) {
					_ctx->frameNumber += 2;
				} else if (FROM_LE_32(pAni[_ctx->frameNumber].op) == ANI_ADJUSTXY) {
					_ctx->frameNumber += 3;
				} else {
					// ANI_STOP, ANI_HIDE, ANI_HFLIP,
					// ANI_VFLIP, ANI_HVFLIP, default
					_ctx->frameNumber++;
				}
			}
		}

		switch (FROM_LE_32(pAni[_ctx->frameNumber].op)) {
		case ANI_END:
			// Stop this sample if repeating
			if (_ctx->sampleNumber && _ctx->bLooped)
				_vm->_sound->stopSpecSample(_ctx->sampleNumber, 0);
			_ctx->bFinished = true;
			break;

		case ANI_JUMP:
			_ctx->frameNumber++;

			assert((int32)FROM_LE_32(pAni[_ctx->frameNumber].op) < 0);

			_ctx->frameNumber += FROM_LE_32(pAni[_ctx->frameNumber].op);

			assert(_ctx->frameNumber >= 0);
			continue;

		case ANI_STOP:
			// Stop this sample
			if (_ctx->sampleNumber)
				_vm->_sound->stopSpecSample(_ctx->sampleNumber, 0);
			break;

		case ANI_HIDE:
			// No op
			break;

		case ANI_HFLIP:
		case ANI_VFLIP:
		case ANI_HVFLIP:
			_ctx->frameNumber++;
			continue;

		case ANI_ADJUSTX:
		case ANI_ADJUSTY:
			_ctx->frameNumber += 2;
			continue;

		case ANI_ADJUSTXY:
			_ctx->frameNumber += 3;
			continue;

		default:
			// Stop this sample
			if (_ctx->sampleNumber)
				_vm->_sound->stopSpecSample(_ctx->sampleNumber, 0);

			_ctx->sampleNumber = FROM_LE_32(pAni[_ctx->frameNumber++].op);
			if (_ctx->sampleNumber > 0)
				_ctx->bLooped = false;
			else {
				_ctx->sampleNumber = ~_ctx->sampleNumber;
				_ctx->bLooped = true;
			}
			x = (short)(FROM_LE_32(pAni[_ctx->frameNumber].op) >> 16);
			y = (short)(FROM_LE_32(pAni[_ctx->frameNumber].op) & 0xffff);

			if (x == 0)
				x = -1;

			_vm->_sound->playSample(_ctx->sampleNumber, 0, _ctx->bLooped, x, y, PRIORITY_SCRIPT,
					Audio::Mixer::kSFXSoundType);

			break;
		}

		CORO_SLEEP(_ctx->speed);
		_ctx->frameNumber++;

		if (_ctx->reelActor && GetActorPresFilm(_ctx->reelActor) != hFilm) {
			// Stop this sample if repeating
			if (_ctx->sampleNumber && _ctx->bLooped)
				_vm->_sound->stopSpecSample(_ctx->sampleNumber, 0);

			_ctx->bFinished = true;
		}

		if (myescEvent && myescEvent != GetEscEvents()) {
			// Stop this sample
			if (_ctx->sampleNumber)
				_vm->_sound->stopSpecSample(_ctx->sampleNumber, 0);

			_ctx->bFinished = true;
		}
	} while (!_ctx->bFinished && _ctx->myNum == g_soundReelNumbers[_ctx->myId]);

	// De-register - if not been replaced
	if (_ctx->myNum == g_soundReelNumbers[_ctx->myId])
		DeRegisterSoundReel(hFilm, column);

	CORO_END_CODE;
}

static void ResSoundReel(CORO_PARAM, const void *param) {
	// get the stuff copied to process when it was created
	int i = *(const int *)param;

	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_ARGS(SoundReel, (CORO_SUBCTX, g_soundReels[i].hFilm, g_soundReels[i].column,
		-1, 0, g_soundReels[i].actorCol));

	CORO_KILL_SELF();
	CORO_END_CODE;
}

static void SoundReelWaitCheck() {
	if (--g_soundReelWait == 0) {
		for (int i = 0; i < MAX_SOUNDREELS; i++) {
			if (g_soundReels[i].hFilm) {
				CoroScheduler.createProcess(PID_REEL, ResSoundReel, &i, sizeof(i));
			}
		}
	}
}

/**
 * - Don't bother if this reel is already playing for this actor.
 * - If explicit co-ordinates, use these, If embedded co-ordinates,
 * leave alone, otherwise use actor's current position.
 * - Moving actors get hidden during this play, other actors get
 * _ctx->replaced by this play.
 * - Column 0 of a film gets its appropriate Z-position, slave columns
 * get slightly bigger Z-positions, in column order.
 * - Play proceeds until the script finishes, another reel starts up for
 * this actor, or the actor gets killed.
 * - If called from an splay(), moving actor's co-ordinates are updated
 * after the play, any walk still in progress will go on from there.
 */
static void t1PlayReel(CORO_PARAM, const PPINIT *ppi) {
	CORO_BEGIN_CONTEXT;
		OBJECT	*pPlayObj;	// Object
		ANIM	thisAnim;	// Animation structure

		bool	mActor;		// Gets set if this is a moving actor
		bool	lifeNoMatter;
		bool	replaced;

		const FREEL *pfreel;	// The 'column' to play
		int		stepCount;
		int		frameCount;
		int		reelActor;
		PMOVER	pActor;
		int tmpX, tmpY;
	CORO_END_CONTEXT(_ctx);

	// FIXME: Avoid non-const global vars
	static int	firstColZ = 0;	// Z-position of column zero
	static int32	fColZfactor = 0;	// Z-factor of column zero's actor

	CORO_BEGIN_CODE(_ctx);

	const MULTI_INIT *pmi;		// MULTI_INIT structure
	bool	bNewMover;	// Gets set if a moving actor that isn't in scene yet

	const FILM *pfilm;

	_ctx->lifeNoMatter = false;
	_ctx->replaced = false;
	_ctx->pActor = NULL;
	bNewMover = false;

	pfilm = (const FILM *)LockMem(ppi->hFilm);
	_ctx->pfreel = &pfilm->reels[ppi->column];

	// Get the MULTI_INIT structure
	pmi = (const MULTI_INIT *)LockMem(FROM_LE_32(_ctx->pfreel->mobj));

	// Save actor's ID
	_ctx->reelActor = (int32)FROM_LE_32(pmi->mulID);

	/**** New (experimental? bit 5/1/95 ****/
	if (!TinselV0 && !actorAlive(_ctx->reelActor))
		return;
	/**** Delete a bit down there if this stays ****/

	UpdateActorEsc(_ctx->reelActor, ppi->escOn, ppi->myescEvent);

	// To handle the play()-talk(), talk()-play(), talk()-talk() and play()-play() scenarios
	if (ppi->hFilm != GetActorLatestFilm(_ctx->reelActor)) {
		// This in not the last film scheduled for this actor

		// It may be the last non-talk film though
		if (ActorIsTalking(_ctx->reelActor))
			SetActorPlayFilm(_ctx->reelActor, ppi->hFilm);	// Revert to this film after talk

		return;
	}
	if (ActorIsTalking(_ctx->reelActor)) {
		// Note: will delete this and there'll be no need to store the talk film!
		if (ppi->hFilm != GetActorTalkFilm(_ctx->reelActor)) {
			SetActorPlayFilm(_ctx->reelActor, ppi->hFilm);	// Revert to this film after talk
			return;
		}
	} else {
		SetActorPlayFilm(_ctx->reelActor, ppi->hFilm);
	}

	// If this reel is already playing for this actor, just forget it.
	if (actorReel(_ctx->reelActor) == _ctx->pfreel)
		return;

	// Poke in the background palette
	PokeInPalette(FROM_LE_32(pmi->hMulFrame));

	// Set up and insert the multi-object
	_ctx->pPlayObj = MultiInitObject(pmi);
	if (!ppi->bTop)
		MultiInsertObject(GetPlayfieldList(FIELD_WORLD), _ctx->pPlayObj);
	else
		MultiInsertObject(GetPlayfieldList(FIELD_STATUS), _ctx->pPlayObj);

	// If co-ordinates are specified, use specified.
	// Otherwise, use actor's position if there are not embedded co-ords.
	// Add this first test for nth columns with offsets
	// in plays with (x,y)
	_ctx->tmpX = ppi->x;
	_ctx->tmpY = ppi->y;
	if (ppi->column != 0 && (pmi->mulX || pmi->mulY)) {
	} else if (_ctx->tmpX != -1 || _ctx->tmpY != -1) {
		MultiSetAniXY(_ctx->pPlayObj, _ctx->tmpX, _ctx->tmpY);
	} else if (!pmi->mulX && !pmi->mulY) {
		GetActorPos(_ctx->reelActor, &_ctx->tmpX, &_ctx->tmpY);
		MultiSetAniXY(_ctx->pPlayObj, _ctx->tmpX, _ctx->tmpY);
	}

	// If it's a moving actor, this hides the moving actor
	// used to do this only if (actorid == 0) - I don't know why
	_ctx->mActor = HideMovingActor(_ctx->reelActor, ppi->sf);

	// If it's a moving actor, get its MOVER structure.
	// If it isn't in the scene yet, get its task running - using
	// Stand() - to prevent a glitch at the end of the play.
	if (_ctx->mActor) {
		_ctx->pActor = GetMover(_ctx->reelActor);
		if (!getMActorState(_ctx->pActor)) {
			CORO_INVOKE_ARGS(Stand, (CORO_SUBCTX, _ctx->reelActor, MAGICX, MAGICY, 0));
			bNewMover = true;
		}
	}

	// Register the fact that we're playing this for this actor
	storeActorReel(_ctx->reelActor, _ctx->pfreel, ppi->hFilm, _ctx->pPlayObj, ppi->column, _ctx->tmpX, _ctx->tmpY);

	/**** Will get rid of this if the above is kept ****/
	// We may be temporarily resuscitating a dead actor
	if (ppi->actorid == 0 && !actorAlive(_ctx->reelActor))
		_ctx->lifeNoMatter = true;

	InitStepAnimScript(&_ctx->thisAnim, _ctx->pPlayObj,  FROM_LE_32(_ctx->pfreel->script), ppi->speed);

	// If first column, set Z position as per
	// Otherwise, column 0's + column number
	// N.B. It HAS been ensured that the first column gets here first
	if (ppi->z != 0) {
		MultiSetZPosition(_ctx->pPlayObj, ppi->z);
		StoreActorZpos(_ctx->reelActor, ppi->z);
	} else if (ppi->bTop) {
		if (ppi->column == 0) {
			firstColZ = Z_TOPPLAY + actorMaskType(_ctx->reelActor);
			MultiSetZPosition(_ctx->pPlayObj, firstColZ);
			StoreActorZpos(_ctx->reelActor, firstColZ);
		} else {
			MultiSetZPosition(_ctx->pPlayObj, firstColZ + ppi->column);
			StoreActorZpos(_ctx->reelActor, firstColZ + ppi->column);
		}
	} else if (ppi->column == 0) {
		if (_ctx->mActor && !bNewMover) {
			// If no path, just use first path in the scene
			if (_ctx->pActor->hCpath == NOPOLY)
				fColZfactor = GetPolyZfactor(FirstPathPoly());
			else
				fColZfactor = GetPolyZfactor(_ctx->pActor->hCpath);
			firstColZ = AsetZPos(_ctx->pPlayObj, MultiLowest(_ctx->pPlayObj), fColZfactor);
		} else {
			switch (actorMaskType(_ctx->reelActor)) {
			case ACT_DEFAULT:
				fColZfactor = 0;
				firstColZ = 2;
				MultiSetZPosition(_ctx->pPlayObj, firstColZ);
				break;
			case ACT_MASK:
				fColZfactor = 0;
				firstColZ = MultiLowest(_ctx->pPlayObj);
				MultiSetZPosition(_ctx->pPlayObj, firstColZ);
				break;
			case ACT_ALWAYS:
				fColZfactor = 10;
				firstColZ = 10000;
				MultiSetZPosition(_ctx->pPlayObj, firstColZ);
				break;
			default:
				fColZfactor = actorMaskType(_ctx->reelActor);
				firstColZ = AsetZPos(_ctx->pPlayObj, MultiLowest(_ctx->pPlayObj), fColZfactor);
				if (firstColZ < 2) {
					// This is an experiment!
					firstColZ = 2;
					MultiSetZPosition(_ctx->pPlayObj, firstColZ);
				}
				break;
			}
		}
		StoreActorZpos(_ctx->reelActor, firstColZ);
	} else {
		if (NoNameFunc(_ctx->reelActor, bNewMover) > fColZfactor) {
			fColZfactor = NoNameFunc(_ctx->reelActor, bNewMover);
			firstColZ = fColZfactor << 10;
		}
		MultiSetZPosition(_ctx->pPlayObj, firstColZ + ppi->column);
		StoreActorZpos(_ctx->reelActor, firstColZ + ppi->column);
	}

	/*
	 * Play until the script finishes,
	 * another reel starts up for this actor,
	 * or the actor gets killed.
	 */
	_ctx->stepCount = 0;
	_ctx->frameCount = 0;
	do {
		if (_ctx->stepCount++ == 0) {
			_ctx->frameCount++;
			StoreActorSteps(_ctx->reelActor, _ctx->frameCount);
		}
		if (_ctx->stepCount == ppi->speed)
			_ctx->stepCount = 0;

		if (StepAnimScript(&_ctx->thisAnim) == ScriptFinished)
			break;

		int x, y;
		GetAniPosition(_ctx->pPlayObj, &x, &y);
		StoreActorPos(_ctx->reelActor, x, y);

		CORO_SLEEP(1);

		if (actorReel(_ctx->reelActor) != _ctx->pfreel) {
			_ctx->replaced = true;
			break;
		}

		if (ActorEsc(_ctx->reelActor) && ActorEev(_ctx->reelActor) != GetEscEvents())
			break;

	} while (_ctx->lifeNoMatter || actorAlive(_ctx->reelActor));

	// Register the fact that we're NOT playing this for this actor
	if (actorReel(_ctx->reelActor) == _ctx->pfreel)
		storeActorReel(_ctx->reelActor, NULL, 0, NULL, 0, 0, 0);

	// Ditch the object
	if (!ppi->bTop)
		MultiDeleteObject(GetPlayfieldList(FIELD_WORLD), _ctx->pPlayObj);
	else
		MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), _ctx->pPlayObj);

	if (_ctx->mActor) {
		if (!_ctx->replaced)
			unHideMovingActor(_ctx->reelActor);	// Restore moving actor

		// Update it's co-ordinates if this is an splay()
		if (ppi->splay)
			restoreMovement(_ctx->reelActor);
	}
	CORO_END_CODE;
}

/**
 * - Don't bother if this reel is already playing for this actor.
 * - If explicit co-ordinates, use these, If embedded co-ordinates,
 * leave alone, otherwise use actor's current position.
 * - Moving actors get hidden during this play, other actors get
 * replaced by this play.
 * - Column 0 of a film gets its appropriate Z-position, slave columns
 * get slightly bigger Z-positions, in column order.
 * - Play proceeds until the script finishes, another reel starts up for
 * this actor, or the actor gets killed.
 * - If called from an splay(), moving actor's co-ordinates are updated
 * after the play, any walk still in progress will go on from there.
 * @param x				Co-ordinates from the play(), set to (-1, -1) if none
 * @param y				Co-ordinates from the play(), set to (-1, -1) if none
 * @param bRestore		Normally False, set if from restore
 * @param speed			Film speed
 * @param hFilm			The 'film'
 * @param column		Column number, first column = 0
 */
static void t2PlayReel(CORO_PARAM, int x, int y, bool bRestore, int speed, SCNHANDLE hFilm,
		int column, int myescEvent, bool bTop) {
	CORO_BEGIN_CONTEXT;
		bool bReplaced;
		bool bGotHidden;
		int	stepCount;
		int	frameCount;
		bool bEscapedAlready;
		bool	bPrinciple;	// true if this is the first column in a film for one actor
		bool	bRelative;	// true if relative specified in script

		FREEL		*pFreel;
		MULTI_INIT	*pmi;	// MULTI_INIT structure
		POBJECT		pPlayObj;	// Object
		ANIM		thisAnim;	// Animation structure

		int	reelActor;			// Which actor this reel belongs to
		PMOVER	pMover;			// set if it's a moving actor
		bool	bNewMover;		// Gets set if a moving actor that isn't in scene yet

		int	filmNumber;
		int	myZ;				// Remember for hide/unhide
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->bReplaced = false;
	_ctx->bGotHidden = false;
	_ctx->stepCount = 0;
	_ctx->frameCount = 0;

	_ctx->bEscapedAlready = false;

	// Get the reel and MULTI_INIT structure
	_ctx->pFreel = GetReel(hFilm, column);
	_ctx->pmi = (MULTI_INIT *)LockMem(FROM_LE_32(_ctx->pFreel->mobj));

	if ((int32)FROM_LE_32(_ctx->pmi->mulID) == -2) {
		CORO_INVOKE_ARGS(SoundReel, (CORO_SUBCTX, hFilm, column, speed, myescEvent,
			FROM_LE_32(_ctx->pmi->otherFlags) & OTH_RELATEDACTOR));
		return;
	}

	// Save actor's ID
	_ctx->reelActor = FROM_LE_32(_ctx->pmi->mulID);

	UpdateActorEsc(_ctx->reelActor, myescEvent);

	// To handle the play()-talk(), talk()-play(), talk()-talk() and play()-play() scenarios
	if (hFilm != GetActorLatestFilm(_ctx->reelActor)) {
		// This in not the last film scheduled for this actor

		// It may be the last non-talk film though
		if (ActorIsTalking(_ctx->reelActor))
			SetActorPlayFilm(_ctx->reelActor, hFilm);	// Revert to this film after talk

		return;
	}
	if (ActorIsTalking(_ctx->reelActor)) {
		// Note: will delete this and there'll be no need to store the talk film!
		if (hFilm != GetActorTalkFilm(_ctx->reelActor)) {
			SetActorPlayFilm(_ctx->reelActor, hFilm);	// Revert to this film after talk
			return;
		}
	} else {
		SetActorPlayFilm(_ctx->reelActor, hFilm);
	}

	// Register the film for this actor
	if (hFilm != GetActorPresFilm(_ctx->reelActor)) {
		_ctx->bPrinciple = true;
		StoreActorPresFilm(_ctx->reelActor, hFilm, x, y);
	} else {
		_ctx->bPrinciple = false;

		// If this reel is already playing for this actor, just forget it.
		if (ActorReelPlaying(_ctx->reelActor, column))
			return;
	}

	/*
	 * Insert the object
	 */
	// Poke in the background palette
	PokeInPalette(_ctx->pmi);

	// Set ghost bit if wanted
	if (ActorIsGhost(_ctx->reelActor)) {
		assert(FROM_LE_32(_ctx->pmi->mulFlags) == DMA_WNZ || FROM_LE_32(_ctx->pmi->mulFlags) == (DMA_WNZ | DMA_GHOST));
		_ctx->pmi->mulFlags = TO_LE_32(FROM_LE_32(_ctx->pmi->mulFlags) | DMA_GHOST);
	}

	// Set up and insert the multi-object
	_ctx->pPlayObj = MultiInitObject(_ctx->pmi);
	if (!bTop)
		MultiInsertObject(GetPlayfieldList(FIELD_WORLD), _ctx->pPlayObj);
	else
		MultiInsertObject(GetPlayfieldList(FIELD_STATUS), _ctx->pPlayObj);

	/*
	 * More action for moving actors
	*/
	_ctx->pMover = GetMover(_ctx->reelActor);
	if (_ctx->pMover != NULL) {
		HideMover(_ctx->pMover);

		if (!MoverIs(_ctx->pMover)) {
			// Used to do a Stand here to prevent glitches

			_ctx->bNewMover = true;
		} else
			_ctx->bNewMover = false;
	}

	// Register the reel for this actor
	StoreActorReel(_ctx->reelActor, column, _ctx->pPlayObj);

	_ctx->filmNumber = GetActorFilmNumber(_ctx->reelActor);

	/*
	 * Sort out x and y
	 */
	assert( ((FROM_LE_32(_ctx->pmi->otherFlags) & OTH_RELATIVE) && !(FROM_LE_32(_ctx->pmi->otherFlags) & OTH_ABSOLUTE))
		|| ((FROM_LE_32(_ctx->pmi->otherFlags) & OTH_ABSOLUTE) && !(FROM_LE_32(_ctx->pmi->otherFlags) & OTH_RELATIVE)) );

	_ctx->bRelative = FROM_LE_32(_ctx->pmi->otherFlags) & OTH_RELATIVE;

	if (_ctx->bRelative) {
		// Use actor's position. If (x, y) specified, move the actor.
		if (x == -1 && y == -1)
			GetActorPos(_ctx->reelActor, &x, &y);
		else
			StoreActorPos(_ctx->reelActor, x, y);
	} else if (x == -1 && y == -1)
		x = y = 0;		// Use (0,0) if no specified

	// Add embedded co-ords
	MultiSetAniXY(_ctx->pPlayObj, x + FROM_LE_32(_ctx->pmi->mulX), y + FROM_LE_32(_ctx->pmi->mulY));

	/*
	 * Sort out z
	 */
	if (bRestore) {
		_ctx->myZ = GetActorZpos(_ctx->reelActor, column);

		SoundReelWaitCheck();
	} else {
		// FIXME: Avoid non-const global vars
		static int baseZposn;		// Z-position of column zero
		static uint32 baseZfact;	// Z-factor of column zero's actor

		// N.B. It HAS been ensured that the first column gets here first

		if ((int32)FROM_LE_32(_ctx->pmi->mulZ) != -1) {
			// Z override in script

			baseZfact = FROM_LE_32(_ctx->pmi->mulZ);
			baseZposn = (baseZfact << ZSHIFT) + MultiLowest(_ctx->pPlayObj);
			if (bTop)
				baseZposn += Z_TOPPLAY;
		} else if (column == 0
				|| GetZfactor(_ctx->reelActor, _ctx->pMover, _ctx->bNewMover) > baseZfact) {
			// Subsequent columns are based on this one

			baseZfact = GetZfactor(_ctx->reelActor, _ctx->pMover, _ctx->bNewMover);
			baseZposn = (baseZfact << ZSHIFT) + MultiLowest(_ctx->pPlayObj);
			if (bTop)
				baseZposn += Z_TOPPLAY;
		}
		_ctx->myZ = baseZposn + column;
	}
	MultiSetZPosition(_ctx->pPlayObj, _ctx->myZ);
	StoreActorZpos(_ctx->reelActor, _ctx->myZ, column);

	/*
	 * Play until the script finishes,
	 * another reel starts up for this actor,
	 * or the actor gets killed.
	 */
	InitStepAnimScript(&_ctx->thisAnim, _ctx->pPlayObj, FROM_LE_32(_ctx->pFreel->script), speed);

	if (bRestore || (ActorEsc(_ctx->reelActor) == true &&
				ActorEev(_ctx->reelActor) != GetEscEvents())) {
		// From restore, step to jump or end
		SkipFrames(&_ctx->thisAnim, -1);
	}

	for (;;) {
		if (_ctx->stepCount++ == 0) {
			_ctx->frameCount++;
			StoreActorSteps(_ctx->reelActor, _ctx->frameCount);
		}
		if (_ctx->stepCount == speed)
			_ctx->stepCount = 0;

		if (_ctx->bPrinciple && AboutToJumpOrEnd(&_ctx->thisAnim))
			IncLoopCount(_ctx->reelActor);

		if (StepAnimScript(&_ctx->thisAnim) == ScriptFinished)
			break;

		if (_ctx->bRelative) {
			GetAniPosition(_ctx->pPlayObj, &x, &y);
			StoreActorPos(_ctx->reelActor, x, y);
		}

		if (_ctx->bGotHidden) {
			if (!ActorHidden(_ctx->reelActor)) {
				MultiSetZPosition(_ctx->pPlayObj, _ctx->myZ);
				_ctx->bGotHidden = false;
			}
		} else {
			if (ActorHidden(_ctx->reelActor)) {
				MultiSetZPosition(_ctx->pPlayObj, -1);
				_ctx->bGotHidden = true;
			}
		}

		CORO_SLEEP(1);

		if (GetActorFilmNumber(_ctx->reelActor) != _ctx->filmNumber) {
			_ctx->bReplaced = true;
			break;
		}

		if (ActorEsc(_ctx->reelActor) == true && ActorEev(_ctx->reelActor) != GetEscEvents()) {
			if (!_ctx->bEscapedAlready) {
				SkipFrames(&_ctx->thisAnim, -1);
				_ctx->bEscapedAlready = true;
			}

//WHY???		UpdateActorEsc(reelActor, GetEscEvents());
// The above line of code, not commented out would fix the coffee pot flash
// but why was it commented out?
// The extra boolean is used instead, 'cos it's release week and I want to play it safe!
		}
	}

	// Register the fact that we're NOT playing this for this actor
	NotPlayingReel(_ctx->reelActor, _ctx->filmNumber, column);

	// Ditch the object
	if (!bTop)
		MultiDeleteObject(GetPlayfieldList(FIELD_WORLD), _ctx->pPlayObj);
	else
		MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), _ctx->pPlayObj);

	// Restore moving actor is nessesary
	if (_ctx->pMover != NULL && _ctx->bPrinciple && !_ctx->bReplaced)
		UnHideMover(_ctx->pMover);

	CORO_END_CODE;
}

/**
 * Run all animations that comprise the play film.
 */
static void PlayProcess(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	const PPINIT *ppi = (const PPINIT *)param;
	CORO_BEGIN_CODE(_ctx);

	if (TinselV2)
		CORO_INVOKE_ARGS(t2PlayReel, (CORO_SUBCTX, ppi->x, ppi->y, ppi->bRestore, ppi->speed,
			ppi->hFilm, ppi->column, ppi->myescEvent, ppi->bTop));
	else
		CORO_INVOKE_1(t1PlayReel, ppi);

	CORO_END_CODE;
}

// *******************************************************


// To handle the play()-talk(), talk()-play(), talk()-talk() and play()-play() scenarios
void NewestFilm(SCNHANDLE film, const FREEL *reel) {
	const MULTI_INIT *pmi;		// MULTI_INIT structure

	// Get the MULTI_INIT structure
	pmi = (const MULTI_INIT *)LockMem(FROM_LE_32(reel->mobj));

	if (!TinselV2 || ((int32)FROM_LE_32(pmi->mulID) != -2))
		SetActorLatestFilm((int32)FROM_LE_32(pmi->mulID), film);
}

// *******************************************************

/**
 * Start up a play process for each column in a film.
 *
 * NOTE: The processes are started in reverse order so that the first
 *   column's process kicks in first.
 */
void PlayFilm(CORO_PARAM, SCNHANDLE hFilm, int x, int y, int actorid, bool splay, bool sfact, bool escOn,
			  int myescEvent, bool bTop) {
	assert(hFilm != 0); // Trying to play NULL film
	const FILM *pFilm;

	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	pFilm = (const FILM *)LockMem(hFilm);
	PPINIT ppi;

	// Now allowed empty films!
	if (pFilm->numreels == 0)
		return;                 // Nothing to do!

	ppi.hFilm = hFilm;
	ppi.x = x;
	ppi.y = y;
	ppi.z = 0;
	ppi.bRestore = false;
	ppi.speed = (ONE_SECOND / FROM_LE_32(pFilm->frate));
	ppi.actorid = actorid;
	ppi.splay = splay;
	ppi.bTop = bTop;
	ppi.sf = sfact;
	ppi.escOn = escOn;
	ppi.myescEvent = myescEvent;

	// Start display process for each reel in the film
	for (int i = FROM_LE_32(pFilm->numreels) - 1; i >= 0; i--) {
		NewestFilm(hFilm, &pFilm->reels[i]);

		ppi.column = i;
		CoroScheduler.createProcess(PID_REEL, PlayProcess, &ppi, sizeof(PPINIT));
	}

	if (TinselV2) {
		// Let it all kick in and position this process
		// down the process list from the playing process(es)
		// This ensures something
		CORO_GIVE_WAY;

		if (myescEvent && myescEvent != GetEscEvents())
			CoroScheduler.rescheduleAll();
	}

	CORO_END_CODE;
}

void PlayFilm(CORO_PARAM, SCNHANDLE hFilm, int x, int y, int myescEvent, bool bTop) {
	PlayFilm(coroParam, hFilm, x, y, 0, false, false, false, myescEvent, bTop);
}

/**
 * Start up a play process for each slave column in a film.
 * Play the first column directly from the parent process.
 */
void PlayFilmc(CORO_PARAM, SCNHANDLE hFilm, int x, int y, int actorid, bool splay, bool sfact,
			   bool escOn, int myescEvent, bool bTop) {
	CORO_BEGIN_CONTEXT;
		PPINIT ppi;
		int i;
		int loopCount;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	assert(hFilm != 0); // Trying to play NULL film
	const FILM *pFilm;

	pFilm = (const FILM *)LockMem(hFilm);

	// Now allowed empty films!
	if (pFilm->numreels == 0)
		return;                 //  Already played to completion!

	_ctx->ppi.hFilm = hFilm;
	_ctx->ppi.x = x;
	_ctx->ppi.y = y;
	_ctx->ppi.z = 0;
	_ctx->ppi.bRestore = false;
	_ctx->ppi.speed = (ONE_SECOND / FROM_LE_32(pFilm->frate));
	_ctx->ppi.actorid = actorid;
	_ctx->ppi.splay = splay;
	_ctx->ppi.bTop = bTop;
	_ctx->ppi.sf = sfact;
	_ctx->ppi.escOn = escOn;
	_ctx->ppi.myescEvent = myescEvent;

	// Start display process for each secondary reel in the film in Tinsel 1,
	// or all of them in Tinsel 2
	for (int i = FROM_LE_32(pFilm->numreels) - 1; i >= (TinselV2 ? 0 : 1); i--) {
		NewestFilm(hFilm, &pFilm->reels[i]);

		_ctx->ppi.column = i;
		CoroScheduler.createProcess(PID_REEL, PlayProcess, &_ctx->ppi, sizeof(PPINIT));
	}

	if (TinselV2) {
		// Let it all kick in and position this 'waiting' process
		// down the process list from the playing process(es)
		// This ensures immediate return when the reel finishes
		CORO_GIVE_WAY;

		_ctx->i = ExtractActor(hFilm);
		_ctx->loopCount = GetLoopCount(_ctx->i);

		// Wait until film changes or loop count increases
		while (GetActorPresFilm(_ctx->i) == hFilm && GetLoopCount(_ctx->i) == _ctx->loopCount) {
			if (myescEvent && myescEvent != GetEscEvents()) {
				CoroScheduler.rescheduleAll();
				break;
			}

			CORO_SLEEP(1);
		}
	} else {
		// For Tinsel 1, launch the primary reel
		NewestFilm(hFilm, &pFilm->reels[0]);

		_ctx->ppi.column = 0;
		CORO_INVOKE_1(t1PlayReel, &_ctx->ppi);
	}

	CORO_END_CODE;
}

/**
 * Start up a play process for a particular column in a film.
 *
 * NOTE: This is specifically for actors during a Tinsel 1 restore scene.
 */
void RestoreActorReels(SCNHANDLE hFilm, short reelnum, short z, int x, int y) {
	assert(!TinselV2);
	const FILM *pfilm = (const FILM *)LockMem(hFilm);
	PPINIT ppi;

	ppi.hFilm = hFilm;
	ppi.x = x;
	ppi.y = y;
	ppi.z = z;
	ppi.speed = (ONE_SECOND / FROM_LE_32(pfilm->frate));
	ppi.actorid = 0;
	ppi.splay = false;
	ppi.bTop = false;
	ppi.bRestore = true;
	ppi.sf = 0;
	ppi.column = reelnum;
	ppi.myescEvent = 0;

	ppi.escOn = false;
	ppi.myescEvent = GetEscEvents();

	assert(pfilm->numreels);

	NewestFilm(hFilm, &pfilm->reels[reelnum]);

	// Start display process for the reel
	CoroScheduler.createProcess(PID_REEL, PlayProcess, &ppi, sizeof(ppi));
}

/**
 * Start up a play process for a particular column in a film.
 *
 * NOTE: This is specifically for actors during a Tinsel 2 restore scene.
 */
void RestoreActorReels(SCNHANDLE hFilm, int actor, int x, int y) {
	assert(TinselV2);
	FILM *pFilm = (FILM *)LockMem(hFilm);
	PPINIT ppi;

	int i;
	FREEL *pFreel;
	PMULTI_INIT	pmi;		// MULTI_INIT structure

	ppi.hFilm = hFilm;
	ppi.x = (short)x;
	ppi.y = (short)y;
	ppi.bRestore = true;
	ppi.speed = (short)(ONE_SECOND/FROM_LE_32(pFilm->frate));
	ppi.bTop = false;
	ppi.myescEvent = 0;

	// Search backwards for now as later column will be the one
	for (i = (int)FROM_LE_32(pFilm->numreels) - 1; i >= 0; i--) {
		pFreel = &pFilm->reels[i];
		pmi = (PMULTI_INIT) LockMem(FROM_LE_32(pFreel->mobj));
		if ((int32)FROM_LE_32(pmi->mulID) == actor) {
			ppi.column = (short)i;
			NewestFilm(hFilm, &pFilm->reels[i]);

			// Start display process for the reel
			CoroScheduler.createProcess(PID_REEL, PlayProcess, &ppi, sizeof(ppi));

			g_soundReelWait++;
		}
	}
}

/**
 * Get the actor id from a film (column 0)
 */
int ExtractActor(SCNHANDLE hFilm) {
	const FILM *pFilm = (const FILM *)LockMem(hFilm);
	const FREEL *pReel = &pFilm->reels[0];
	const MULTI_INIT *pmi = (const MULTI_INIT *)LockMem(FROM_LE_32(pReel->mobj));
	return (int)FROM_LE_32(pmi->mulID);
}

} // End of namespace Tinsel
