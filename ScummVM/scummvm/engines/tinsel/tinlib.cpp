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
 * Glitter library functions.
 *
 * In the main called only from PCODE.C
 * Function names are the same as Glitter code function names.
 *
 * To ensure exclusive use of resources and exclusive control responsibilities.
 */

#define BODGE

#include "common/coroutines.h"
#include "tinsel/actors.h"
#include "tinsel/background.h"
#include "tinsel/bmv.h"
#include "tinsel/config.h"
#include "tinsel/cursor.h"
#include "tinsel/drives.h"
#include "tinsel/dw.h"
#include "tinsel/events.h"
#include "tinsel/faders.h"
#include "tinsel/film.h"
#include "tinsel/font.h"
#include "tinsel/graphics.h"
#include "tinsel/handle.h"
#include "tinsel/dialogs.h"
#include "tinsel/mareels.h"
#include "tinsel/move.h"
#include "tinsel/multiobj.h"
#include "tinsel/music.h"
#include "tinsel/object.h"
#include "tinsel/palette.h"
#include "tinsel/pcode.h"
#include "tinsel/pid.h"
#include "tinsel/play.h"
#include "tinsel/polygons.h"
#include "tinsel/rince.h"
#include "tinsel/savescn.h"
#include "tinsel/sched.h"
#include "tinsel/scn.h"
#include "tinsel/scroll.h"
#include "tinsel/sound.h"
#include "tinsel/strres.h"
#include "tinsel/sysvar.h"
#include "tinsel/text.h"
#include "tinsel/timers.h"		// For ONE_SECOND constant
#include "tinsel/tinlib.h"
#include "tinsel/tinsel.h"
#include "tinsel/token.h"

#include "common/textconsole.h"

namespace Tinsel {

//----------------- EXTERNAL GLOBAL DATA --------------------

// In DOS_DW.C
extern bool g_bRestart;		// restart flag - set to restart the game
extern bool g_bHasRestarted;	// Set after a restart

// In PCODE.CPP
extern bool g_bNoPause;

// In DOS_MAIN.C
// TODO/FIXME: From dos_main.c: "Only used on PSX so far"
//int clRunMode = 0;

//----------------- EXTERNAL FUNCTIONS ---------------------

// in BG.CPP
extern void ChangePalette(SCNHANDLE hPal);

// in PDISPLAY.CPP
extern void EnableTags();
extern void DisableTags();
bool DisableTagsIfEnabled();
extern void setshowstring();

// in SAVELOAD.CPP
extern int NewestSavedGame();

// in SCENE.CPP
extern void setshowpos();
extern int g_sceneCtr;

// in TINSEL.CPP
extern void SetCdChangeScene(SCNHANDLE hScene);
extern void SetHookScene(SCNHANDLE scene, int entrance, int transition);
extern void SetNewScene(SCNHANDLE scene, int entrance, int transition);
extern void UnHookScene();
extern void SuspendHook();
extern void UnSuspendHook();

#ifdef BODGE
// In HANDLE.CPP
bool ValidHandle(SCNHANDLE offset);

// In SCENE.CPP
SCNHANDLE GetSceneHandle();
#endif

//----------------- GLOBAL GLOBAL DATA --------------------

// FIXME: Avoid non-const global vars

bool g_bEnableMenu;

static bool g_bInstantScroll = false;
static bool g_bEscapedCdPlay = false;


//----------------- LOCAL DEFINES --------------------

#define JAP_TEXT_TIME	(2*ONE_SECOND)

/*----------------------------------------------------------------------*\
|*                      Library Procedure and Function codes            *|
\*----------------------------------------------------------------------*/

enum MASTER_LIB_CODES {
	ACTORATTR, ACTORBRIGHTNESS, ACTORDIRECTION, ACTORPALETTE, ACTORPRIORITY, ACTORREF,
	ACTORRGB, ACTORSCALE, ACTORSON, ACTORXPOS, ACTORYPOS, ADDHIGHLIGHT,
	ADDINV, ADDINV1, ADDINV2, ADDOPENINV, ADDTOPIC, AUXSCALE, BACKGROUND, BLOCKING,
	CALLACTOR, CALLGLOBALPROCESS, CALLOBJECT, CALLPROCESS, CALLSCENE, CALLTAG,
	CAMERA, CDCHANGESCENE, CDDOCHANGE, CDENDACTOR, CDLOAD, CDPLAY, CLEARHOOKSCENE,
	CLOSEINVENTORY, CONTROL, CONVERSATION, CONVTOPIC, CURSOR, CURSORXPOS, CURSORYPOS,
	CUTSCENE, DECCONVW, DECCSTRINGS, DECCURSOR, DECFLAGS, DECINV1, DECINV2, DECINVW,
	DECLARELANGUAGE, DECLEAD, DECSCALE, DECTAGFONT, DECTALKFONT, DELICON,
	DELINV, DELTOPIC, DIMMUSIC, DROP, DROPEVERYTHING, DROPOUT, EFFECTACTOR, ENABLEMENU,
	ENDACTOR, ESCAPE, ESCAPEOFF, ESCAPEON, EVENT, FACETAG, FADEIN, FADEMIDI,
	FADEOUT, FRAMEGRAB, FREEZECURSOR, GETINVLIMIT, GHOST, GLOBALVAR, GRABMOVIE, HAILSCENE,
	HASRESTARTED, HAVE, HELDOBJECT, HIDEACTOR, HIDEBLOCK, HIDEEFFECT, HIDEPATH,
	HIDEREFER, HIDETAG, HOLD, HOOKSCENE, IDLETIME, ININVENTORY, INSTANTSCROLL, INVDEPICT,
	INVENTORY, INVPLAY, INWHICHINV, KILLACTOR, KILLBLOCK, KILLEXIT, KILLGLOBALPROCESS,
	KILLPROCESS, KILLTAG, LOCALVAR, MOVECURSOR, MOVETAG, MOVETAGTO, NEWSCENE,
	NOBLOCKING, NOPAUSE, NOSCROLL, OBJECTHELD, OFFSET, OTHEROBJECT, PAUSE, PLAY, PLAYMIDI,
	PLAYMOVIE, PLAYMUSIC, PLAYRTF, PLAYSAMPLE, POINTACTOR, POINTTAG, POSTACTOR, POSTGLOBALPROCESS,
	POSTOBJECT, POSTPROCESS, POSTTAG, PREPARESCENE, PRINT, PRINTCURSOR, PRINTOBJ, PRINTTAG,
	QUITGAME, RANDOM, RESETIDLETIME, RESTARTGAME, RESTORESCENE, RESTORE_CUT,
	RESUMELASTGAME, RUNMODE, SAMPLEPLAYING, SAVESCENE, SAY, SAYAT, SCALINGREELS,
	SCANICON, SCREENXPOS, SCREENYPOS, SCROLL, SCROLLPARAMETERS, SENDACTOR, SENDGLOBALPROCESS,
	SENDOBJECT, SENDPROCESS, SENDTAG, SETACTOR, SETBLOCK, SETBRIGHTNESS, SETEXIT, SETINVLIMIT,
	SETINVSIZE, SETLANGUAGE, SETPALETTE, SETSYSTEMREEL, SETSYSTEMSTRING, SETSYSTEMVAR,
	SETTAG, SETTIMER, SHELL, SHOWACTOR, SHOWBLOCK, SHOWEFFECT, SHOWMENU, SHOWPATH,
	SHOWPOS, SHOWREFER, SHOWSTRING, SHOWTAG, SPLAY, STAND, STANDTAG, STARTGLOBALPROCESS,
	STARTPROCESS, STARTTIMER, STOPMIDI, STOPSAMPLE, STOPWALK, SUBTITLES, SWALK, SWALKZ,
	SYSTEMVAR, TAGACTOR, TAGTAGXPOS, TAGTAGYPOS, TAGWALKXPOS, TAGWALKYPOS, TALK, TALKAT,
	TALKATS, TALKATTR, TALKPALETTEINDEX, TALKRGB, TALKVIA, TEMPTAGFONT, TEMPTALKFONT,
	THISOBJECT, THISTAG, TIMER, TOPIC, TOPPLAY, TOPWINDOW, TRANSLUCENTINDEX,
	TRYPLAYSAMPLE, UNDIMMUSIC, UNHOOKSCENE, UNTAGACTOR, VIBRATE, WAITFRAME, WAITKEY,
	WAITSCROLL, WAITTIME, WALK, WALKED, WALKEDPOLY, WALKEDTAG, WALKINGACTOR, WALKPOLY,
	WALKTAG, WALKXPOS, WALKYPOS, WHICHCD, WHICHINVENTORY, ZZZZZZ,
	HIGHEST_LIBCODE
};

static const MASTER_LIB_CODES DW1DEMO_CODES[] = {
	ACTORREF, ACTORXPOS, ACTORYPOS, ADDTOPIC, ADDINV1, ADDINV2, AUXSCALE, BACKGROUND,
	CAMERA, CONTROL, CONVERSATION, CONVTOPIC, HIGHEST_LIBCODE, CURSORXPOS, CURSORYPOS,
	DECCONVW, DECCURSOR, DECTAGFONT, DECINVW, DECINV1, DECINV2, DECLEAD, DELICON,
	DELINV, EVENT, HIGHEST_LIBCODE, HELDOBJECT, HIDEACTOR, ININVENTORY, HIGHEST_LIBCODE,
	INVENTORY, HIGHEST_LIBCODE, KILLACTOR, KILLBLOCK, KILLTAG, SCREENXPOS,
	HIGHEST_LIBCODE, MOVECURSOR, NEWSCENE, NOSCROLL, OBJECTHELD, OFFSET, HIGHEST_LIBCODE,
	PLAY, PLAYSAMPLE, PREPARESCENE, PRINT, PRINTOBJ, PRINTTAG, RESTORESCENE, SAVESCENE,
	SCANICON, SCROLL, SETACTOR, SETBLOCK, HIGHEST_LIBCODE, SETTAG, SETTIMER, SHOWPOS,
	SPLAY, STAND, STANDTAG, STOPWALK, HIGHEST_LIBCODE, SWALK, TAGACTOR, TALK,
	SCREENYPOS, UNTAGACTOR, VIBRATE, WAITKEY, WAITTIME, WALK, WALKINGACTOR, WALKPOLY,
	WALKTAG, RANDOM, TIMER
};

static const MASTER_LIB_CODES DW1_CODES[] = {
	ACTORATTR, ACTORDIRECTION, ACTORREF, ACTORSCALE, ACTORXPOS,
	ACTORYPOS, ADDTOPIC, ADDINV1, ADDINV2, ADDOPENINV, AUXSCALE,
	BACKGROUND, CAMERA, CLOSEINVENTORY, CONTROL, CONVERSATION,
	CONVTOPIC, CURSORXPOS, CURSORYPOS, DECCONVW, DECCURSOR,
	DECINV1, DECINV2, DECINVW, DECLEAD, DECTAGFONT,
	DECTALKFONT, DELICON, DELINV, EFFECTACTOR, ESCAPE, EVENT,
	GETINVLIMIT, HELDOBJECT, HIDEACTOR, ININVENTORY, INVDEPICT,
	INVENTORY, KILLACTOR, KILLBLOCK, KILLEXIT, KILLTAG, SCREENXPOS,
	MOVECURSOR, NEWSCENE, NOSCROLL, OBJECTHELD, OFFSET, PAUSE,
	PLAY, PLAYMIDI, PLAYSAMPLE, PREPARESCENE, PRINT, PRINTOBJ,
	PRINTTAG, RANDOM, RESTORESCENE, SAVESCENE, SCALINGREELS,
	SCANICON, SCROLL, SETACTOR, SETBLOCK, SETEXIT, SETINVLIMIT,
	SETPALETTE, SETTAG, SETTIMER, SHOWPOS, SHOWSTRING, SPLAY,
	STAND, STANDTAG, STOPWALK, SWALK, TAGACTOR, TALK, TALKATTR, TIMER,
	SCREENYPOS, TOPPLAY, TOPWINDOW, UNTAGACTOR, VIBRATE, WAITKEY,
	WAITTIME, WALK, WALKED, WALKINGACTOR, WALKPOLY, WALKTAG,
	WHICHINVENTORY, ACTORSON, CUTSCENE, HOOKSCENE, IDLETIME,
	RESETIDLETIME, TALKAT, UNHOOKSCENE, WAITFRAME,	DECCSTRINGS,
	STOPMIDI, STOPSAMPLE, TALKATS, DECFLAGS, FADEMIDI,
	CLEARHOOKSCENE, SETINVSIZE, INWHICHINV, NOBLOCKING,
	SAMPLEPLAYING, TRYPLAYSAMPLE, ENABLEMENU, RESTARTGAME, QUITGAME,
	FRAMEGRAB, PLAYRTF, CDPLAY, CDLOAD, HASRESTARTED, RESTORE_CUT,
	RUNMODE, SUBTITLES, SETLANGUAGE,
	HIGHEST_LIBCODE
};

static const MASTER_LIB_CODES DW2DEMO_CODES[] = {
	ACTORBRIGHTNESS, ACTORDIRECTION, ACTORPALETTE, ACTORPRIORITY,
	ACTORREF, ACTORRGB, ACTORSCALE, ACTORXPOS, ACTORYPOS,
	ADDHIGHLIGHT, ADDINV, ADDINV1, ADDINV2, ADDOPENINV, ADDTOPIC,
	BACKGROUND, CALLACTOR, CALLGLOBALPROCESS, CALLOBJECT,
	CALLPROCESS, CALLSCENE, CALLTAG, CAMERA, CDCHANGESCENE,
	CDDOCHANGE, CDLOAD, CDPLAY, CLEARHOOKSCENE, CLOSEINVENTORY,
	CONTROL, CONVERSATION, CURSOR, CURSORXPOS, CURSORYPOS,
	DECCONVW, DECCURSOR, DECFLAGS, DECINV1, DECINV2, DECINVW,
	DECLEAD, DECSCALE, DECTAGFONT, DECTALKFONT, DELTOPIC,
	DIMMUSIC, DROP, DROPOUT, EFFECTACTOR, ENABLEMENU, ENDACTOR,
	ESCAPEOFF, ESCAPEON, EVENT, FACETAG, FADEIN, FADEOUT, FRAMEGRAB,
	FREEZECURSOR, GETINVLIMIT, GHOST, GLOBALVAR, HASRESTARTED,
	HAVE, HELDOBJECT, HIDEACTOR, HIDEBLOCK, HIDEEFFECT, HIDEPATH,
	HIDEREFER, HIDETAG, HOLD, HOOKSCENE, IDLETIME, INSTANTSCROLL,
	INVENTORY, INVPLAY, INWHICHINV, KILLACTOR, KILLGLOBALPROCESS,
	KILLPROCESS, LOCALVAR, MOVECURSOR, MOVETAG, MOVETAGTO, NEWSCENE,
	NOBLOCKING, NOPAUSE, NOSCROLL, OFFSET, OTHEROBJECT, PAUSE, PLAY,
	PLAYMUSIC, PLAYRTF, PLAYSAMPLE, POINTACTOR, POINTTAG, POSTACTOR,
	POSTGLOBALPROCESS, POSTOBJECT, POSTPROCESS, POSTTAG, PRINT,
	PRINTCURSOR, PRINTOBJ, PRINTTAG, QUITGAME, RANDOM, RESETIDLETIME,
	RESTARTGAME, RESTORESCENE, RUNMODE, SAVESCENE, SAY, SAYAT,
	SCALINGREELS, SCREENXPOS, SCREENYPOS, SCROLL, SCROLLPARAMETERS,
	SENDACTOR, SENDGLOBALPROCESS, SENDOBJECT, SENDPROCESS, SENDTAG,
	SETBRIGHTNESS, SETINVLIMIT, SETINVSIZE, SETLANGUAGE, SETPALETTE,
	SETSYSTEMSTRING, SETSYSTEMVAR, SHELL, SHOWACTOR, SHOWBLOCK,
	SHOWEFFECT, SHOWPATH, SHOWREFER, SHOWTAG, STAND, STANDTAG,
	STARTGLOBALPROCESS, STARTPROCESS, STARTTIMER, STOPWALK, SUBTITLES,
	SWALK, SYSTEMVAR, TAGTAGXPOS, TAGTAGYPOS, TAGWALKXPOS, TAGWALKYPOS,
	TALK, TALKAT, TALKPALETTEINDEX, TALKRGB, TALKVIA, THISOBJECT,
	THISTAG, TIMER, TOPIC, TOPPLAY, TOPWINDOW, TRANSLUCENTINDEX,
	UNDIMMUSIC, UNHOOKSCENE, WAITFRAME, WAITKEY, WAITSCROLL, WAITTIME,
	WALK, WALKED, WALKEDPOLY, WALKEDTAG, WALKINGACTOR, WALKPOLY,
	WALKTAG, WALKXPOS, WALKYPOS, WHICHCD, WHICHINVENTORY,
	HIGHEST_LIBCODE
};

static const MASTER_LIB_CODES DW2_CODES[] = {
	ACTORBRIGHTNESS, ACTORDIRECTION, ACTORPALETTE, ACTORPRIORITY,
	ACTORREF, ACTORRGB, ACTORSCALE, ACTORXPOS, ACTORYPOS,
	ADDHIGHLIGHT, ADDINV, ADDINV1, ADDINV2, ADDOPENINV, ADDTOPIC,
	BACKGROUND, CALLACTOR, CALLGLOBALPROCESS, CALLOBJECT,
	CALLPROCESS, CALLSCENE, CALLTAG, CAMERA, CDCHANGESCENE,
	CDDOCHANGE, CDLOAD, CDPLAY, CLEARHOOKSCENE, CLOSEINVENTORY,
	CONTROL, CONVERSATION, CURSOR, CURSORXPOS, CURSORYPOS,
	DECCONVW, DECCURSOR, DECFLAGS, DECINV1, DECINV2, DECINVW,
	DECLEAD, DECSCALE, DECTAGFONT, DECTALKFONT, DELTOPIC,
	DIMMUSIC, DROP, DROPOUT, EFFECTACTOR, ENABLEMENU, ENDACTOR,
	ESCAPEOFF, ESCAPEON, EVENT, FACETAG, FADEIN, FADEOUT, FRAMEGRAB,
	FREEZECURSOR, GETINVLIMIT, GHOST, GLOBALVAR, GRABMOVIE,
	HASRESTARTED, HAVE, HELDOBJECT, HIDEACTOR, HIDEBLOCK, HIDEEFFECT,
	HIDEPATH, HIDEREFER, HIDETAG, HOLD, HOOKSCENE, IDLETIME,
	INSTANTSCROLL, INVENTORY, INVPLAY, INWHICHINV, KILLACTOR,
	KILLGLOBALPROCESS, KILLPROCESS, LOCALVAR, MOVECURSOR, MOVETAG,
	MOVETAGTO, NEWSCENE, NOBLOCKING, NOPAUSE, NOSCROLL, OFFSET,
	OTHEROBJECT, PAUSE, PLAY, PLAYMUSIC, PLAYRTF, PLAYSAMPLE,
	POINTACTOR, POINTTAG, POSTACTOR, POSTGLOBALPROCESS, POSTOBJECT,
	POSTPROCESS, POSTTAG, PRINT, PRINTCURSOR, PRINTOBJ, PRINTTAG,
	QUITGAME, RANDOM, RESETIDLETIME, RESTARTGAME, RESTORESCENE,
	RUNMODE, SAVESCENE, SAY, SAYAT, SCALINGREELS, SCREENXPOS,
	SCREENYPOS, SCROLL, SCROLLPARAMETERS, SENDACTOR, SENDGLOBALPROCESS,
	SENDOBJECT, SENDPROCESS, SENDTAG, SETBRIGHTNESS, SETINVLIMIT,
	SETINVSIZE, SETLANGUAGE, SETPALETTE, SETSYSTEMSTRING, SETSYSTEMVAR,
	SHELL, SHOWACTOR, SHOWBLOCK, SHOWEFFECT, SHOWPATH, SHOWREFER,
	SHOWTAG, STAND, STANDTAG, STARTGLOBALPROCESS, STARTPROCESS,
	STARTTIMER, STOPWALK, SUBTITLES, SWALK, SYSTEMVAR, TAGTAGXPOS,
	TAGTAGYPOS, TAGWALKXPOS, TAGWALKYPOS, TALK, TALKAT, TALKPALETTEINDEX,
	TALKRGB, TALKVIA, THISOBJECT, THISTAG, TIMER, TOPIC, TOPPLAY,
	TOPWINDOW, TRANSLUCENTINDEX, UNDIMMUSIC, UNHOOKSCENE, WAITFRAME,
	WAITKEY, WAITSCROLL, WAITTIME, WALK, WALKED, WALKEDPOLY, WALKEDTAG,
	WALKINGACTOR, WALKPOLY, WALKTAG, WALKXPOS, WALKYPOS, WHICHCD,
	WHICHINVENTORY, ZZZZZZ, SWALKZ, DROPEVERYTHING, BLOCKING, STOPSAMPLE,
	CDENDACTOR, DECLARELANGUAGE, RESUMELASTGAME, SHOWMENU, TEMPTALKFONT,
	TEMPTAGFONT, PLAYMOVIE, HAILSCENE, SETSYSTEMREEL,
	HIGHEST_LIBCODE
};

//----------------- LOCAL GLOBAL DATA --------------------

// FIXME: Avoid non-const global vars

// Saved cursor co-ordinates for control(on) to restore cursor position
// as it was at control(off).
// They are global so that MoveCursor(..) has a net effect if it
// precedes control(on).
static int g_controlX = 0, g_controlY = 0;

static int g_offtype = 0;			// used by Control()
static uint32 g_lastValue = 0;	// used by RandomFn()
static int g_scrollNumber = 0;	// used by scroll()

static bool g_bNotPointedRunning = false;	// Used in Printobj and PrintObjPointed

//----------------- FORWARD REFERENCES --------------------

static int HeldObject();
static void PostTag(CORO_PARAM, int tagno, TINSEL_EVENT event, HPOLYGON hp, int myEscape);
void ResetIdleTime();
static void SendTag(CORO_PARAM, int tagno, TINSEL_EVENT event, HPOLYGON hp, int myEscape, bool *result);
static void StandTag(int actor, HPOLYGON hp);
void StopMidiFn();
void StopSample(int sample = -1);
static void StopWalk(int actor);
static void WaitScroll(CORO_PARAM, int myescEvent);
void Walk(CORO_PARAM, int actor, int x, int y, SCNHANDLE film, int hold, bool igPath,
		  int zOverride, bool escOn, int myescTime);

//----------------- SUPPORT FUNCTIONS --------------------

/**
 * For Scroll() and Offset(), work out top left for a
 * given screen position.
 */
static void DecodeExtreme(EXTREME extreme, int *px, int *py) {
	int	Loffset, Toffset;

	PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);

	switch (extreme) {
	case EX_BOTTOM:
		*px = Loffset;
		*py = BgHeight() - SCREEN_HEIGHT;
		break;
	case EX_BOTTOMLEFT:
		*px = 0;
		*py = BgHeight() - SCREEN_HEIGHT;
		break;
	case EX_BOTTOMRIGHT:
		*px = BgWidth() - SCREEN_WIDTH;
		*py = BgHeight() - SCREEN_HEIGHT;
		break;
	case EX_LEFT:
		*px = 0;
		*py = Toffset;
		break;
	case EX_RIGHT:
		*px = BgWidth() - SCREEN_WIDTH;
		*py = Toffset;
		break;
	case EX_TOP:
		*px = Loffset;
		*py = 0;
		break;
	case EX_TOPLEFT:
		*px = *py = 0;
		break;
	case EX_TOPRIGHT:
		*px = BgWidth() - SCREEN_WIDTH;
		*py = 0;
		break;
	default:
		break;
	}
}

static void KillSelf(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_KILL_SELF();

	CORO_END_CODE;
}

struct SCROLL_MONITOR {
	int	x;
	int	y;
	int	thisScroll;
	int	myEscape;
};
typedef SCROLL_MONITOR *PSCROLL_MONITOR;

/**
 * Monitor a scrolling, allowing Escape to interrupt it
 */
static void ScrollMonitorProcess(CORO_PARAM, const void *param) {
	int		Loffset, Toffset;
	const SCROLL_MONITOR *psm = (const SCROLL_MONITOR *)param;

	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	do {
		CORO_SLEEP(1);

		// give up if have been superseded
		if (psm->thisScroll != g_scrollNumber)
			break;

		// If ESCAPE is pressed...
		if (psm->myEscape != GetEscEvents()) {
			// Instant completion!
			Offset(EX_USEXY, psm->x, psm->y);
			break;
		}

		PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);

	} while (Loffset != psm->x || Toffset != psm->y);

	CORO_END_CODE;
}

/**
 * NOT A LIBRARY FUNCTION
 *
 * Poke supplied color into the DAC queue.
 */
void SetTextPal(COLORREF col) {
	SetTalkColorRef(col);
	UpdateDACqueue(TalkColor(), col);
}

/**
 * Work out a time depending on length of string and
 * subtitle speed modification.
 */
static int TextTime(char *pTstring) {
	if (isJapanMode())
		return JAP_TEXT_TIME;
	else if (!_vm->_config->_textSpeed)
		return strlen(pTstring) + ONE_SECOND;
	else
		return strlen(pTstring) + ONE_SECOND + (_vm->_config->_textSpeed * 5 * ONE_SECOND) / 100;
}

/**
 * KeepOnScreen
 */
void KeepOnScreen(POBJECT pText, int *pTextX, int *pTextY) {
	int	shift;

	// Not off the left
	shift = MultiLeftmost(pText);
	if (shift < 0) {
		MultiMoveRelXY(pText, - shift, 0);
		*pTextX -= shift;
	}

	// Not off the right
	shift = MultiRightmost(pText);
	if (shift > SCREEN_WIDTH) {
		MultiMoveRelXY(pText, SCREEN_WIDTH - shift, 0);
		*pTextX += SCREEN_WIDTH - shift;
	}

	// Not off the top
	shift = MultiHighest(pText);
	if (shift < 0) {
		MultiMoveRelXY(pText, 0, - shift);
		*pTextY -= shift;
	}

	// Not off the bottom
	shift = MultiLowest(pText);
	if (shift > SCREEN_BOX_HEIGHT2) {
		MultiMoveRelXY(pText, 0, SCREEN_BOX_HEIGHT2 - shift);
		*pTextX += SCREEN_WIDTH - shift;
	}
}

/**
 * Waits until the specified process is finished
 */
static void FinishWaiting(CORO_PARAM, const INT_CONTEXT *pic, bool *result = NULL) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	while (pic->resumeCode == RES_WAITING)
		CORO_SLEEP(1);

	if (result)
		*result = pic->resumeCode == RES_FINISHED;
	CORO_END_CODE;
}

void TinGetVersion(WHICH_VER which, char *buffer, int length) {

	if (length > VER_LEN)
		length = VER_LEN;

	char *cptr = (char *)FindChunk(MASTER_SCNHANDLE, CHUNK_TIME_STAMPS);

	switch (which)	{
	case VER_GLITTER:
		memcpy(buffer, cptr, length);
		break;

	case VER_COMPILE:
		memcpy(buffer, cptr + VER_LEN, length);
		break;
	}
}

/********************************************************************\
|*****			Library functions								*****|
\********************************************************************/

/**
 * Set actor's attributes.
 * - currently only the text color.
 */
static void ActorAttr(int actor, int r1, int g1, int b1) {
	storeActorAttr(actor, r1, g1, b1);
}

/**
 * Behave as if actor has walked into a polygon with given brughtness.
 */
void ActorBrightness(int actor, int brightness) {
	PMOVER pMover = GetMover(actor);

	assert(pMover != NULL);
	assert(brightness >= 0 && brightness <= 10);

	MoverBrightness(pMover, brightness);
}

/**
 * Return a moving actor's current direction.
 */
static int ActorDirection(int actor) {
	PMOVER pMover = GetMover(actor);
	assert(pMover);

	return (int)GetMoverDirection(pMover);
}

/**
 * Set actor's palette details for path brightnesses
 */
void ActorPalette(int actor, int startColor, int length) {
	PMOVER pMover = GetMover(actor);
	assert(pMover);

	StoreMoverPalette(pMover, startColor, length);
}

/**
 * Set actor's Z-factor.
 */
static void ActorPriority(int actor, int zFactor) {
	SetActorZfactor(actor, zFactor);
}

/**
 * Set actor's text color.
 */
static void ActorRGB(int actor, COLORREF color) {
	SetActorRGB(actor, color);
}

/**
 * Return the actor's scale.
 */
static int ActorScale(int actor) {
	PMOVER pMover = GetMover(actor);
	assert(pMover);

	return (int)GetMoverScale(pMover);
}

/**
 * Returns the x or y position of an actor.
 */
static int ActorPos(int xory, int actor) {
	int x, y;

	GetActorPos(actor, &x, &y);
	return (xory == ACTORXPOS) ? x : y;
}

/**
 * Make all actors alive at the start of each scene.
 */
static void ActorsOn() {
	setactorson();
}

/**
 * Adds an icon to the conversation window.
 */
static void AddTopic(int icon) {
	AddToInventory(INV_CONV, icon, false);
}

/**
 * Place the object in inventory 1 or 2.
 */
static void AddInv(int invno, int object) {
	// illegal inventory number
	assert(invno == INV_1 || invno == INV_2 || invno == INV_OPEN || invno == INV_DEFAULT);

	AddToInventory(invno, object, false);
}

/**
 * Define an actor's walk and stand reels for an auxilliary scale.
 */
static void AuxScale(int actor, int scale, SCNHANDLE *rp) {
	PMOVER pMover = GetMover(actor);
	assert(pMover);

	int j;
	for (j = 0; j < 4; ++j)
		pMover->walkReels[scale-1][j] = *rp++;
	for (j = 0; j < 4; ++j)
		pMover->standReels[scale-1][j] = *rp++;
	for (j = 0; j < 4; ++j)
		pMover->talkReels[scale-1][j] = *rp++;
}

/**
 * Defines the background image for a scene.
 */
static void Background(CORO_PARAM, SCNHANDLE bfilm) {
	StartupBackground(coroParam, bfilm);
}

/**
 * Disable dynamic blocking for current scene.
 */
void Blocking(bool onOrOff) {
	SetSysVar(ISV_NO_BLOCKING, !onOrOff);
}

/**
 * Sets focus of the scroll process.
 */
static void Camera(int actor) {
	ScrollFocus(actor);
}

/**
 * Sets the CD Change Scene
 */

static void CdChangeScene(SCNHANDLE hScene) {
	SetCdChangeScene(hScene);
}

/**
 * CdDoChange
 */
void CdDoChange(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (!GotoCD())
		return;

	CORO_INVOKE_0(CdCD);

	CdHasChanged();

	CORO_END_CODE;
}

/**
 * CdEndActor("actor")
 */
void CdEndActor(int	actor, int	myEscape) {
	PMOVER	pMover;			// for if it's a moving actor

	// Only do it if escaped!
	if (myEscape && myEscape != GetEscEvents()) {
		// End current graphic
		dwEndActor(actor);

		// un-hide movers
		pMover = GetMover(actor);
		if (pMover)
			UnHideMover(pMover);
	}
}

/**
 * A CDPLAY() is imminent.
 */
static void CDload(SCNHANDLE start, SCNHANDLE next, int myEscape) {
	assert(start && next && start != next); // cdload() fault

	if (TinselV2) {
		if (myEscape && myEscape != GetEscEvents()) {
			g_bEscapedCdPlay = true;
			return;
		}

		LoadExtraGraphData(start, next);
	}
}

/**
 * Clear the hooked scene (if any)
 */
static void ClearHookScene() {
	SetHookScene(0, 0, TRANS_DEF);
}

/**
 * Guess what.
 */
static void CloseInventory() {
	KillInventory();
}

/**
 * Turn off cursor and take control from player - and variations on the	 theme.
 *  OR Restore cursor and return control to the player.
 */
void Control(int param) {
	if (TinselV2) {
		if (param)
			ControlOn();
		else {
			ControlOff();

			switch (WhichInventoryOpen()) {
			case INV_1:
			case INV_2:
			case INV_MENU:
				KillInventory();
				break;
			default:
				break;
			}
		}

		return;
	}

	// Tinsel 1 handling code
	g_bEnableMenu = false;

	switch (param) {
	case CONTROL_STARTOFF:
		GetControlToken();	// Take control
		DisableTags();			// Switch off tags
		DwHideCursor();			// Blank out cursor
		g_offtype = param;
		break;

	case CONTROL_OFF:
	case CONTROL_OFFV:
	case CONTROL_OFFV2:
		if (TestToken(TOKEN_CONTROL)) {
			GetControlToken();	// Take control

			DisableTags();			// Switch off tags
			GetCursorXYNoWait(&g_controlX, &g_controlY, true);	// Store cursor position

			// There may be a button timing out
			GetToken(TOKEN_LEFT_BUT);
			FreeToken(TOKEN_LEFT_BUT);
		}

		if (g_offtype == CONTROL_STARTOFF)
			GetCursorXYNoWait(&g_controlX, &g_controlY, true);	// Store cursor position

		g_offtype = param;

		if (param == CONTROL_OFF)
			DwHideCursor();		// Blank out cursor
		else if (param == CONTROL_OFFV) {
			UnHideCursor();
			FreezeCursor();
		} else if (param == CONTROL_OFFV2) {
			UnHideCursor();
		}
		break;

	case CONTROL_ON:
		if (g_offtype != CONTROL_OFFV2 && g_offtype != CONTROL_STARTOFF)
			SetCursorXY(g_controlX, g_controlY);// ... where it was

		FreeControlToken();	// Release control

		if (!InventoryActive())
			EnableTags();		// Tags back on

		RestoreMainCursor();		// Re-instate cursor...
	}
}

/**
 * Open or close the conversation window.
 */
static void Conversation(CORO_PARAM, int fn, HPOLYGON hp, int actor, bool escOn, int myEscape) {
	assert(hp != NOPOLY); // conversation() must (currently) be called from a polygon code block
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (fn == CONV_END) {
		// Close down conversation
		CloseDownConv();
	} else if ((fn == CONV_TOP) || (fn == CONV_DEF) || (fn == CONV_BOTTOM)) {
		// TOP of screen, Default (i.e. TOP of screen), or BOTTOM of screen

		// If waiting is enabled, wait for ongoing scroll
		if (TinselV2 && SysVar(SV_CONVERSATIONWAITS))
			CORO_INVOKE_1(WaitScroll, myEscape);

		// Don't do it if it's not wanted
		if (escOn && myEscape != GetEscEvents())
			return;

		// Don't do it if already in a conversation
		if (IsConvWindow())
			return;

		KillInventory();

		if (TinselV2) {
			// If this is from a tag polygon, get the associated
			// actor (the one the polygon is named after), if any.
			if (!actor) {
				actor = GetTagPolyId(hp);
				if (actor & ACTORTAG_KEY)
					actor &= ~ACTORTAG_KEY;
				else
					actor = 0;
			}

			// Top or bottom; tag polygon or tagged actor
			SetConvDetails((CONV_PARAM)fn, hp, actor);
		} else {
			convPos(fn);
			ConvPoly(hp);
		}

		PopUpInventory(INV_CONV);	// Conversation window
		ConvAction(INV_OPENICON);	// CONVERSATION event
	}

	CORO_END_CODE;
}

/**
 * Add icon to conversation window's permanent default list.
 */
static void ConvTopic(int icon) {
	PermaConvIcon(icon);
}

/**
 * Cursor(on/off)
 */
void Cursor(int onoff) {
	if (onoff) {
		// Re-instate cursor
		UnHideCursor();
	} else {
		// Blank out cursor
		DwHideCursor();
	}
}

/**
 * Returns the x or y position of the cursor.
 */
static int CursorPos(int xory) {
	int x, y;

	GetCursorXY(&x, &y, true);
	return (xory == CURSORXPOS) ? x : y;
}

/**
 * Declare conversation window.
 */
static void DecConvW(SCNHANDLE text, int MaxContents, int MinWidth, int MinHeight,
			int StartWidth, int StartHeight, int MaxWidth, int MaxHeight) {
	idec_convw(text, MaxContents, MinWidth, MinHeight,
			StartWidth, StartHeight, MaxWidth, MaxHeight);
}

/**
 * Declare config strings.
 */
static void DecCStrings(SCNHANDLE *tp) {
	setConfigStrings(tp);
}

/**
 * Declare cursor's reels.
 */
static void DecCursor(SCNHANDLE hFilm) {
	DwInitCursor(hFilm);
}

/**
 * Declare the language flags.
 */
static void DecFlags(SCNHANDLE hFilm) {
	setFlagFilms(hFilm);
}

/**
 * Declare inventory 1's parameters.
 */
static void DecInv1(SCNHANDLE text, int MaxContents,
		int MinWidth, int MinHeight,
		int StartWidth, int StartHeight,
		int MaxWidth, int MaxHeight) {
	idec_inv1(text, MaxContents, MinWidth, MinHeight,
			StartWidth, StartHeight, MaxWidth, MaxHeight);
}

/**
 * Declare inventory 2's parameters.
 */
static void DecInv2(SCNHANDLE text, int MaxContents,
		int MinWidth, int MinHeight,
		int StartWidth, int StartHeight,
		int MaxWidth, int MaxHeight) {
	idec_inv2(text, MaxContents, MinWidth, MinHeight,
			StartWidth, StartHeight, MaxWidth, MaxHeight);
}

/**
 * Declare the bits that the inventory windows are constructed from.
 */
static void DecInvW(SCNHANDLE hf) {
	setInvWinParts(hf);
}

/**
 * DeclareLanguage
 */
static void DeclareLanguage(int languageId, SCNHANDLE hDescription, SCNHANDLE hFlagFilm) {
	LanguageFacts(languageId, hDescription, hFlagFilm);
}

/**
 * Declare lead actor.
 * @param id		Actor Id
 * @param rp		Walk and stand reels for all the regular scales (v1 only)
 * @param text		Tag text (v1 only)
 */
static void DecLead(uint32 id, SCNHANDLE *rp = 0, SCNHANDLE text = 0) {
	PMOVER	pMover;		// Moving actor structure

	if (TinselV2) {
		// Tinsel 2 only specifies the lead actor Id
		SetLeadId(id);
		RegisterMover(id);

	} else {

		Tag_Actor(id, text, TAG_DEF);	// The lead actor is automatically tagged
		SetLeadId(id);			// Establish this as the lead
		RegisterMover(id);			// Establish as a moving actor

		pMover = GetMover(id);		// Get moving actor structure
		assert(pMover);

		// Store all those reels
		int i, j;
		for (i = 0; i < 5; ++i) {
			for (j = 0; j < 4; ++j)
				pMover->walkReels[i][j] = *rp++;
			for (j = 0; j < 4; ++j)
				pMover->standReels[i][j] = *rp++;
			for (j = 0; j < 4; ++j)
				pMover->talkReels[i][j] = *rp++;
		}


		for (i = NUM_MAINSCALES; i < TOTAL_SCALES; i++) {
			for (j = 0; j < 4; ++j) {
				pMover->walkReels[i][j] = pMover->walkReels[4][j];
				pMover->standReels[i][j] = pMover->standReels[2][j];
				pMover->talkReels[i][j] = pMover->talkReels[4][j];
			}
		}
	}
}

/**
 * DecScale("actor", scale, 12*"reel")
 * Define an actor's walk and stand reels for a scale.
 */
static void DecScale(int actor, int scale,
		SCNHANDLE wkl, SCNHANDLE wkr, SCNHANDLE wkf, SCNHANDLE wka,
		SCNHANDLE stl, SCNHANDLE str, SCNHANDLE stf, SCNHANDLE sta,
		SCNHANDLE tal, SCNHANDLE tar, SCNHANDLE taf, SCNHANDLE taa) {
	PMOVER pMover = GetMover(actor);
	assert(pMover);

	SetWalkReels(pMover, scale, wkl, wkr, wkf, wka);
	SetStandReels(pMover, scale, stl, str, stf, sta);
	SetTalkReels(pMover, scale, tal, tar, taf, taa);
}

/**
 * Declare the text font.
 */
static void DecTagFont(SCNHANDLE hf) {
	SetTagFontHandle(hf);		// Store the font handle
	if (TinselV0)
		SetTalkFontHandle(hf);	// Also re-use for talk text
}

/**
 * Declare the text font.
 */
static void DecTalkFont(SCNHANDLE hf) {
	SetTalkFontHandle(hf);		// Store the font handle
}

/**
 * Remove an icon from the conversation window.
 */
static void DelIcon(int icon) {
	RemFromInventory(INV_CONV, icon);
}

/**
 * Delete the object from inventory 1 or 2.
 */
static void DelInv(int object) {
	if (!RemFromInventory(INV_1, object))		// Remove from inventory 1...
		RemFromInventory(INV_2, object);		// ...or 2 (whichever)

	DropItem(object);			// Stop holding it
}

/**
 * DelTopic
 */
static void DelTopic(int icon) {
	RemFromInventory(INV_CONV, icon);
}

/**
 * DimMusic
 */
static void DimMusic() {
	_vm->_pcmMusic->dim(true);
}

/**
 * Delete the object from inventory 1 or 2.
 */
static void Drop(int object) {
	if (object == -1)
		object = HeldObject();

	if (!RemFromInventory(INV_1, object))	// Remove from inventory 1...
		RemFromInventory(INV_2, object);	// ...or 2 (whichever)

	DropItem(object);			// Stop holding it
}

/**
 * Delete all objects from inventory 1 and 2.
 */
static void DropEverything() {
	HoldItem(NOOBJECT, false);

	ClearInventory(INV_1);
	ClearInventory(INV_2);
}

/**
 * EnableMenu
 */
static void EnableMenu() {
	g_bEnableMenu = true;
}

/**
 * Kill an actor's current graphics.
 */
static void EndActor(int actor) {
	dwEndActor(actor);
}

/**
 * Get the actor to look at the polygon.
 * If the actor is at the tag, do a StandTag().
 */
static void FaceTag(int actor, HPOLYGON hp) {
	PMOVER	pMover;		// Moving actor structure
	int	nowx, nowy;
	int	nodex, nodey;

	assert(hp != NOPOLY);

	/*
	 * Get which moving actor it is
	 */
	pMover = GetMover(actor);
	assert(pMover);
	if (MoverHidden(pMover))
		return;

	/*
	 * Stop the actor
	 */
	StopWalk(actor);

	/*
	 * Face the tag
	 */
	// See where node is and where actor is
	GetPolyNode(hp, &nodex, &nodey);
	GetActorPos(actor, &nowx, &nowy);

	if (nowx == nodex && nowy == nodey) {
		// Stood at the tag, don't face in silly direction
		StandTag(actor, hp);
	} else {
		// Look towards polygon
		GetPolyMidBottom(hp, &nodex, &nodey);
		SetMoverDirection(pMover, GetDirection(nowx, nowy,
						nodex, nodey,
						GetMoverDirection(pMover),
						NOPOLY, YB_X1_5));
		SetMoverStanding(pMover);
	}
}

/**
 * FadeIn
 */
static void FadeIn() {
	FadeInMedium(NULL);
}

/**
 * FadeOut
 */
static void FadeOut() {
	FadeOutMedium(NULL);
}

/**
 * FadeMidi(in/out)
 */
static void FadeMidi(CORO_PARAM, int inout) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	assert(inout == FM_IN || inout == FM_OUT);

	// To prevent compiler complaining
	if (inout == FM_IN || inout == FM_OUT)
		CORO_SLEEP(1);
	CORO_END_CODE;
}

/**
 * Freeze the cursor, or not.
 */
static void FreezeCursor(bool bFreeze) {
	DoFreezeCursor(bFreeze);
}

/**
 * Guess what.
 */
static int GetInvLimit(int invno) {
	return InvGetLimit(invno);
}

/**
 * Ghost
 */
static void Ghost(int actor, int tColor, int tPalOffset) {
	SetSysVar(ISV_GHOST_ACTOR, actor);
	SetSysVar(ISV_GHOST_COLOR,  tColor);
	SetSysVar(ISV_GHOST_BASE, tPalOffset);
	CreateGhostPalette(BgPal());
}

/**
 *
 */
static void HailScene(SCNHANDLE scene) {
	DoHailScene(scene);
}

/**
 * Returns TRUE if the game has been restarted, FALSE if not.
 */
static bool HasRestarted() {
	return g_bHasRestarted;
}

/**
 * See if an object is in the inventory.
 */
int Have(int object) {
	return (InventoryPos(object) != NOOBJECT);
}

/**
 * Returns which object is currently held.
 */
static int HeldObject() {
	return WhichItemHeld();
}

/**
 * Hides the specified actor
 */
static void HideActorFn(CORO_PARAM, int ano) {
	HideActor(coroParam, ano);
}

/**
 * Turn a blocking polygon off.
 */
static void HideBlock(int block) {
	DisableBlock(block);
}

/**
 * Turn an effect polygon off.
 */
static void HideEffect(int effect) {
	DisableEffect(effect);
}

/**
 * Turn a path polygon off.
 */
static void HidePath(int path) {
	DisablePath(path);
}

/**
 * Turn a refer polygon off.
 */
static void HideRefer(int refer) {
	DisableRefer(refer);
}

/**
 * Turn a tag polygon off.
 */
static void HideTag(CORO_PARAM, int tag, HPOLYGON hp) {
	// Tag could be zero, meaning calling tag
	DisableTag(coroParam, tag ? tag : GetTagPolyId(hp));
}

/**
 * Hold the specified object.
 */
static void Hold(int object) {
	HoldItem(object, false);
}

/**
 * HookScene(scene, entrance, transition)
 */
void HookScene(SCNHANDLE scene, int entrance, int transition) {
	SetHookScene(scene, entrance, transition);
}

/**
 * IdleTime
 */
static int IdleTime() {
	// If control is off, system is not idle
	if (!ControlIsOn()) {
		// Player doesn't currently have control
		ResetIdleTime();

		return 0;
	} else {
		// Player has control - return time since last event
		int x = getUserEventTime() / ONE_SECOND;

		return x;
	}
}

/**
 * Set flag if InstantScroll(on), reset if InstantScroll(off)
 */
void InstantScroll(int onoff) {
	g_bInstantScroll = (onoff != 0);
}

/**
 * invdepict
 */
static void InvDepict(int object, SCNHANDLE hFilm) {
	SetObjectFilm(object, hFilm);
}

/**
 * See if an object is in the inventory.
 */
int InInventory(int object) {
	return (InventoryPos(object) != INV_NOICON);
}

/**
 * Open an inventory.
 */
static void Inventory(int invno, bool escOn, int myEscape) {
	// Don't do it if it's not wanted
	if (escOn && myEscape != GetEscEvents())
		return;

	assert((invno == INV_1 || invno == INV_2)); // Trying to open illegal inventory

	PopUpInventory(invno);
}

/**
 * Alter inventory object's icon.
 */
static void InvPlay(int object, SCNHANDLE hFilm) {
	SetObjectFilm(object, hFilm);
}

/**
 * See if an object is in the inventory.
 */
static int InWhichInv(int object) {
	if (WhichItemHeld() == object)
		return 0;

	if (IsInInventory(object, INV_1))
		return 1;

	if (IsInInventory(object, INV_2))
		return 2;

	return -1;
}

/**
 * Kill an actor.
 */
static void KillActor(int actor) {
	DisableActor(actor);
}

/**
 * Turn a blocking polygon off.
 */
static void KillBlock(int block) {
	DisableBlock(block);
}

/**
 * Turn an exit off.
 */
static void KillExit(int exit) {
	DisableExit(exit);
}

/**
 * Turn a tag off.
 */
static void KillTag(CORO_PARAM, int tagno) {
	DisableTag(coroParam, tagno);
}

/**
 * Kills the specified global process
 */
static void KillGlobalProcess(uint32 procID) {
	xKillGlobalProcess(procID);
}

/**
 * Kills the specified process
 */
static void KillProcess(uint32 procID) {
	KillSceneProcess(procID);
}

/**
 * Returns the left or top offset of the screen.
 */
static int LToffset(int lort) {
	int Loffset, Toffset;

	PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);
	return (lort == SCREENXPOS) ? Loffset : Toffset;
}

/**
 * Set new cursor position.
 */
static void MoveCursor(int x, int y) {
	SetCursorXY(x, y);

	g_controlX = x;		// Save these values so that
	g_controlY = y;		// control(on) doesn't undo this
}

/**
 * MoveTag(tag, x, y)
 */
void MoveTag(int tag, int x, int y, HPOLYGON hp) {
	// Tag could be zero, meaning calling tag
	MovePolygon(TAG, tag ? tag : GetTagPolyId(hp), x, y);
}

/**
 * MoveTagTo(tag, x, y)
 */
void MoveTagTo(int tag, int x, int y, HPOLYGON hp) {
	// Tag could be zero, meaning calling tag
	MovePolygonTo(TAG, tag ? tag : GetTagPolyId(hp), x, y);
}

/**
 * Triggers change to a new scene.
 */
void NewScene(CORO_PARAM, SCNHANDLE scene, int entrance, int transition) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (TinselV2) {
		if (_vm->_bmv->MoviePlaying()) {
			_vm->_bmv->AbortMovie();
			CORO_SLEEP(2);
		}
	}

	SetNewScene(scene, entrance, transition);

	// Prevent tags and cursor re-appearing
	if (TinselV2)
		ControlStartOff();
	else
		GetControl(CONTROL_STARTOFF);

	if (TinselV1)
		++g_sceneCtr;

	// Prevent code subsequent to this call running before scene changes
	if (CoroScheduler.getCurrentPID() != PID_MASTER_SCR)
		CORO_KILL_SELF();
	CORO_END_CODE;
}

/**
 * Disable dynamic blocking for current scene.
 */
static void NoBlocking() {
	SetNoBlocking(true);
}

/**
 * Define a no-scroll boundary for the current scene.
 */
static void NoScroll(int x1, int y1, int x2, int y2) {
	SetNoScroll(x1, y1, x2, y2);
}

/**
 * Hold the specified object.
 */
static void ObjectHeld(int object) {
	HoldItem(object);
}

/**
 * Set the top left offset of the screen.
 */
void Offset(EXTREME extreme, int x, int y) {
	KillScroll();

	if (TinselV2)
		DecodeExtreme(extreme, &x, &y);

	PlayfieldSetPos(FIELD_WORLD, x, y);
}

/**
 * OtherObject()
 */
int OtherObject(INV_OBJECT *pinvo) {
	assert(pinvo != NULL);

	// return held object or object clicked on - whichever is not the calling object

	// pinvo->id is the calling object
	// WhichItemHeld() gives the held object
	// GetIcon() gives the object clicked on

	assert(GetIcon() == pinvo->id || WhichItemHeld() == pinvo->id);

	if (GetIcon() == pinvo->id)
		return WhichItemHeld();
	else
		return GetIcon();
}

/**
 * Play a film.
 */
static void Play(CORO_PARAM, SCNHANDLE hFilm, int x, int y, int compit, int actorid, bool splay, int sfact,
		  bool escOn, int myEscape, bool bTop) {
	assert(hFilm != 0); // play(): Trying to play NULL film

	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);


	// Don't do CDPlay() for now if already escaped
	if (g_bEscapedCdPlay) {
		g_bEscapedCdPlay = false;
		return;
	}

	// Don't do it if it's not wanted
	if (escOn && myEscape != GetEscEvents())
		return;

	// If this actor is dead, call a stop to the calling process
	if (actorid && !actorAlive(actorid))
		CORO_KILL_SELF();

	// 7/4/95
	if (!escOn)
		myEscape = GetEscEvents();

	if (compit == 1) {
		// Play to completion before returning
		CORO_INVOKE_ARGS(PlayFilmc, (CORO_SUBCTX, hFilm, x, y, actorid, splay, sfact, escOn, myEscape, bTop));
	} else if (compit == 2) {
		error("play(): compit == 2 - please advise John");
	} else {
		// Kick off the play and return.
		CORO_INVOKE_ARGS(PlayFilm, (CORO_SUBCTX, hFilm, x, y, actorid, splay, sfact, escOn, myEscape, bTop));
	}
	CORO_END_CODE;
}

/**
 * Play a film
 */
static void Play(CORO_PARAM, SCNHANDLE hFilm, int x, int y, bool bComplete, int myEscape,
		bool bTop, TINSEL_EVENT event, HPOLYGON hPoly, int taggedActor) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	assert(hFilm != 0);

	// Don't do CdPlay() for now if already escaped
	if (g_bEscapedCdPlay) {
		g_bEscapedCdPlay = false;
		return;
	}

	if (event == TALKING) {
		int	actor;
		if (hPoly == NOPOLY) {
			// Must be a tagged actor

			assert(taggedActor && IsTaggedActor(taggedActor));
			actor = taggedActor;
		} else if (taggedActor == 0) {
			// Must be a polygon with an actor ID
			actor = GetTagPolyId(hPoly);
			assert(actor & ACTORTAG_KEY);
			actor &= ~ACTORTAG_KEY;
		}
		else {
			return;
		}

		SetActorTalking(actor, true);
		SetActorTalkFilm(actor, hFilm);
	}

	if (bComplete) {
		// Play to completion before returning
		CORO_INVOKE_ARGS(PlayFilmc, (CORO_SUBCTX, hFilm, x, y, 0, false, false, myEscape != 0, myEscape, bTop));
	} else {
		// Kick off the play and return.
		CORO_INVOKE_ARGS(PlayFilm, (CORO_SUBCTX, hFilm, x, y, myEscape, bTop));
	}

	CORO_END_CODE;
}


/**
 * Play a midi file.
 */
static void PlayMidi(CORO_PARAM, SCNHANDLE hMidi, int loop, bool complete) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	assert(loop == MIDI_DEF || loop == MIDI_LOOP);

	PlayMidiSequence(hMidi, loop == MIDI_LOOP);

	// This check&sleep was added in DW v2. It was most likely added to
	// ensure that the MIDI song started playing before the next opcode
	// is executed.
	// In DW1, it messes up the script arguments when entering the secret
	// door in the bookshelf in the library, leading to a crash, when the
	// music volume is set to 0.
	if (!MidiPlaying() && TinselV2)
		CORO_SLEEP(1);

	if (complete) {
		while (MidiPlaying())
			CORO_SLEEP(1);
	}
	CORO_END_CODE;
}

/**
 * Plays a movie
 */

static void PlayMovie(CORO_PARAM, SCNHANDLE hFileStem, int myEscape) {
	CORO_BEGIN_CONTEXT;
		int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (myEscape && myEscape != GetEscEvents())
		return;

	// Get rid of the cursor
	for (_ctx->i = 0; _ctx->i < 3; _ctx->i++) {
		DwHideCursor();
		DropCursor();
		CORO_SLEEP(1);
	}

	// They claim to be getting "Can't play two movies at once!" error
	while (_vm->_bmv->MoviePlaying())
		CORO_SLEEP(1);

	// Play the movie
	CORO_INVOKE_2(_vm->_bmv->PlayBMV, hFileStem, myEscape);

	CORO_END_CODE;
}

/**
 * Play some music
 */
static void PlayMusic(int tune) {
	_vm->_pcmMusic->startPlay(tune);
}

/**
 * Play a sample.
 * Tinsel 1 version
 */
static void PlaySample(CORO_PARAM, int sample, bool bComplete, bool escOn, int myEscape) {
	CORO_BEGIN_CONTEXT;
		Audio::SoundHandle handle;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	// Don't play SFX if voice is already playing
	if (_vm->_mixer->hasActiveChannelOfType(Audio::Mixer::kSpeechSoundType))
		return;

	// Don't do it if it's not wanted
	if (escOn && myEscape != GetEscEvents()) {
		_vm->_sound->stopAllSamples();		// Stop any currently playing sample
		return;
	}

	if (_vm->_config->_soundVolume != 0 && _vm->_sound->sampleExists(sample)) {
		_vm->_sound->playSample(sample, Audio::Mixer::kSFXSoundType, &_ctx->handle);

		if (bComplete) {
			while (_vm->_mixer->isSoundHandleActive(_ctx->handle)) {
				// Abort if escapable and ESCAPE is pressed
				if (escOn && myEscape != GetEscEvents()) {
					_vm->_mixer->stopHandle(_ctx->handle);
					break;
				}

				CORO_SLEEP(1);
			}
		}
	} else {
		// Prevent Glitter lock-up
		CORO_SLEEP(1);
	}
	CORO_END_CODE;
}

/**
 * Play a sample
 * Tinsel 2 version
 */
static void PlaySample(CORO_PARAM, int sample, int x, int y, int flags, int myEscape) {
	int	priority;
	CORO_BEGIN_CONTEXT;
		Audio::SoundHandle handle;
		int myEscape;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->myEscape = myEscape;

	// Not escapable if PlaySample(..., s)
	if (flags & PS_SUSTAIN) {
		_ctx->myEscape = 0;
		priority = PRIORITY_SPLAY2;
	} else {
		priority = PRIORITY_SPLAY1;
	}

	// Don't do anything if it's already been escaped
	if (_ctx->myEscape && _ctx->myEscape != GetEscEvents())
		return;

	if (_vm->_config->_soundVolume != 0 && _vm->_sound->sampleExists(sample)) {
		if (x == 0)
			x = -1;

		_vm->_sound->playSample(sample, 0, false, x, y, priority, Audio::Mixer::kSFXSoundType,
			&_ctx->handle);

		if (flags & PS_COMPLETE) {
			while (_vm->_mixer->isSoundHandleActive(_ctx->handle)) {
				// Abort if escapable and ESCAPE is pressed
				if (_ctx->myEscape && _ctx->myEscape != GetEscEvents()) {
					_vm->_mixer->stopHandle(_ctx->handle);
					break;
				}

				CORO_SLEEP(1);
			}
		}
	} else {
		// Prevent Glitter lock-up
		CORO_SLEEP(1);
	}

	CORO_END_CODE;
}

/**
 * Move the cursor to the tagged actor's tag point.
 */
void PointActor(int actor) {
	int	x, y;

	// Only do this if the function is enabled
	if (!SysVar(SV_ENABLEPOINTTAG))
		return;

	assert(IsTaggedActor(actor));

	GetActorTagPos(actor, &x, &y, true);

	_vm->setMousePosition(Common::Point(x, y));
}

/**
 * Move the cursor to the tag's tag point.
 */
static void PointTag(int tagno, HPOLYGON hp) {
	int	x, y;
	SCNHANDLE junk;

	// Only do this if the function is enabled
	if (!SysVar(SV_ENABLEPOINTTAG))
		return;

	// Tag could be zero, meaning calling tag
	if (tagno == 0)
		tagno = GetTagPolyId(hp);

	GetTagTag(GetTagHandle(tagno), &junk, &x, &y);

	_vm->setMousePosition(Common::Point(x, y));
}

/**
 * PostActor("actor", event)
 */
static void PostActor(CORO_PARAM, int actor, TINSEL_EVENT event, HPOLYGON hp,
			   int taggedActor, int myEscape) {
	if (actor == -1) {
		actor = taggedActor;
		assert(hp == NOPOLY && taggedActor);
		assert(IsTaggedActor(actor));
	}

	if (IsTaggedActor(actor)) {
		assert(actor);
		ActorEvent(coroParam, actor, event, false, myEscape);
	} else {
		PostTag(coroParam, actor | ACTORTAG_KEY, event, hp, myEscape);
	}
}

/**
 * PostGlobalProcess(process#, event)
 */
static void PostGlobalProcess(CORO_PARAM, int procId, TINSEL_EVENT event, int myEscape) {
	GlobalProcessEvent(coroParam, procId, event, false, myEscape);
}

/**
 * PostObject(object, event)
 */
static void PostObject(CORO_PARAM, int object, TINSEL_EVENT event, int myEscape) {
	ObjectEvent(coroParam, object, event, false, myEscape);
}

/**
 * PostProcess(process#, event)
 */
static void PostProcess(CORO_PARAM, int procId, TINSEL_EVENT event, int myEscape) {
	SceneProcessEvent(coroParam, procId, event, false, myEscape);
}

/**
 * Posts an event to a specified tag
 */
static void PostTag(CORO_PARAM, int tagno, TINSEL_EVENT event, HPOLYGON hp, int myEscape) {
	// Tag could be zero, meaning calling tag
	if (tagno == 0) {
		assert(hp != NOPOLY);
		PolygonEvent(coroParam, hp, event, 0, false, myEscape);
	} else {
		assert(IsTagPolygon(tagno));
		PolygonEvent(coroParam, GetTagHandle(tagno), event, 0, false, myEscape);
	}
}

/**
 * Trigger pre-loading of a scene's data.
 */
static void PrepareScene(SCNHANDLE scene) {
#ifdef BODGE
	if (!ValidHandle(scene))
		return;
#endif
}

/**
 * Print the given text at the given place for the given time.
 */
static void Print(CORO_PARAM, int x, int y, SCNHANDLE text, int time, bool bSustain, bool escOn, int myEscape) {
	if (TinselV2)
		escOn = myEscape != 0;

	CORO_BEGIN_CONTEXT;
		OBJECT *pText;			// text object pointer
		int	myleftEvent;
		bool bSample;			// Set if a sample is playing
		Audio::SoundHandle handle;
		int timeout;
		int time;
	CORO_END_CONTEXT(_ctx);

	bool	bJapDoPrintText;	// Bodge to get-around Japanese bodge

	CORO_BEGIN_CODE(_ctx);

	_ctx->pText = NULL;
	_ctx->bSample = false;

	// Don't do it if it's not wanted
	if (escOn && myEscape != GetEscEvents())
		return;

	if (!TinselV2) {
		// Kick off the voice sample
		if (_vm->_config->_voiceVolume != 0 && _vm->_sound->sampleExists(text)) {
			_vm->_sound->playSample(text, Audio::Mixer::kSpeechSoundType, &_ctx->handle);
			_ctx->bSample = _vm->_mixer->isSoundHandleActive(_ctx->handle);
		}
	}

	// Get the string
	LoadStringRes(text, TextBufferAddr(), TBUFSZ);

	// Calculate display time
	bJapDoPrintText = false;
	if (time == 0) {
		// This is a 'talky' print
		_ctx->time = TextTime(TextBufferAddr());

		// Cut short-able if sustain was not set
		_ctx->myleftEvent = bSustain ? 0 : GetLeftEvents();
	} else {
		_ctx->time = time * ONE_SECOND;
		_ctx->myleftEvent = (TinselV2 && !bSustain) ? GetLeftEvents() : 0;
		if (isJapanMode())
			bJapDoPrintText = true;
	}

	// Print the text
	if (TinselV2) {
		int Loffset, Toffset;
		PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);
		_ctx->pText = ObjectTextOut(GetPlayfieldList(FIELD_STATUS),
			TextBufferAddr(), 0, x - Loffset, y - Toffset, GetTagFontHandle(),
			TXT_CENTER, 0);
		assert(_ctx->pText);

		// Adjust x, y, or z if necessary
		KeepOnScreen(_ctx->pText, &x, &y);
		if (IsTopWindow())
			MultiSetZPosition(_ctx->pText, Z_TOPW_TEXT);

	} else if (bJapDoPrintText || (!isJapanMode() && (_vm->_config->_useSubtitles || !_ctx->bSample))) {
		int Loffset, Toffset;	// Screen position
		PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);
		_ctx->pText = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), TextBufferAddr(),
					0, x - Loffset, y - Toffset,
					TinselV2 ? GetTagFontHandle() : GetTalkFontHandle(), TXT_CENTER);
		assert(_ctx->pText); // string produced NULL text
		if (IsTopWindow())
			MultiSetZPosition(_ctx->pText, Z_TOPW_TEXT);

		/*
		 * New feature: Don't go off the side of the background
		 */
		int	shift;
		shift = MultiRightmost(_ctx->pText) + 2;
		if (shift >= BgWidth())			// Not off right
			MultiMoveRelXY(_ctx->pText, BgWidth() - shift, 0);
		shift = MultiLeftmost(_ctx->pText) - 1;
		if (shift <= 0)					// Not off left
			MultiMoveRelXY(_ctx->pText, -shift, 0);
		shift = MultiLowest(_ctx->pText);
		if (shift > BgHeight())			// Not off bottom
			MultiMoveRelXY(_ctx->pText, 0, BgHeight() - shift);
	}

	// Give up if nothing printed and no sample
	if (_ctx->pText == NULL && !_ctx->bSample)
		return;

	// Leave it up until time runs out or whatever
	if (TinselV2) {
		do {
			CORO_SLEEP(1);

			// Cancelled?
			if ( (myEscape && myEscape != GetEscEvents())
					|| (!bSustain && LeftEventChange(_ctx->myleftEvent)))
				break;

		} while (_ctx->time-- >= 0);

	} else {
		_ctx->timeout = SAMPLETIMEOUT;
		do {
			CORO_SLEEP(1);

			// Abort if escapable and ESCAPE is pressed
			// Abort if left click - hardwired feature for talky-print!
			// Will be ignored if myleftevent happens to be 0!
			// Abort if sample times out
			if ((escOn && myEscape != GetEscEvents())
			|| (_ctx->myleftEvent && _ctx->myleftEvent != GetLeftEvents())
			|| (_ctx->bSample && --_ctx->timeout <= 0))
				break;

			if (_ctx->bSample) {
				// Wait for sample to end whether or not
				if (!_vm->_mixer->isSoundHandleActive(_ctx->handle)) {
					if (_ctx->pText == NULL || _vm->_config->_textSpeed == DEFTEXTSPEED)				{
						// No text or speed modification - just depends on sample
						break;
					} else {
						// Must wait for time
						_ctx->bSample = false;
					}
				}
			} else {
				// No sample - just depends on time
				if (_ctx->time-- <= 0)
					break;
			}

		} while (1);
	}

	// Delete the text
	if (_ctx->pText != NULL)
		MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), _ctx->pText);
	_vm->_mixer->stopHandle(_ctx->handle);

	CORO_END_CODE;
}


static void PrintObjPointed(CORO_PARAM, const SCNHANDLE text, const INV_OBJECT *pinvo, OBJECT *&pText, const int textx, const int texty, const int item);
static void PrintObjNonPointed(CORO_PARAM, const SCNHANDLE text, const OBJECT *pText);

/**
 * Print the given inventory object's name or whatever.
 */
static void PrintObj(CORO_PARAM, const SCNHANDLE hText, const INV_OBJECT *pinvo, const int event, int myEscape) {
	CORO_BEGIN_CONTEXT;
		OBJECT *pText;		// text object pointer
		int	textx, texty;
		int	item;
		bool bSample;
		int sub;
		Audio::SoundHandle handle;
		int ticks;
		int timeout;
		bool bTookControl;
		int myEscape;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	assert(pinvo != 0); // PrintObj() may only be called from an object code block
	_ctx->myEscape = myEscape;

	if (hText == (SCNHANDLE)-1) {	// 'OFF'
		g_bNotPointedRunning = true;
		return;
	}
	if (hText == (SCNHANDLE)-2) {	// 'ON'
		g_bNotPointedRunning = false;
		return;
	}

	// Don't do it if it's not wanted
	if (TinselV2 && (myEscape) && (myEscape != GetEscEvents()))
		return;

	/*
	* Find out which icon the cursor is over, and where to put the text.
	*/
	GetCursorXY(&_ctx->textx, &_ctx->texty, false);	// Cursor position..
	_ctx->item = InvItem(&_ctx->textx, &_ctx->texty, true);	// ..to text position
	if (_ctx->item == INV_NOICON)
		return;

	/*
	* POINT/other event PrintObj() arbitration...
	*/
	if (event != POINTED) {
		g_bNotPointedRunning = true;	// Get POINTED text to die
		CORO_SLEEP(1);		// Give it chance to
	} else if (!TinselV2)
		g_bNotPointedRunning = false;	// There may have been an OFF without an ON

	// Make multi-ones escape
	if (TinselV2 && (SubStringCount(hText) > 1) && !_ctx->myEscape)
		_ctx->myEscape = GetEscEvents();

	// Loop once for Tinsel 1 strings, and for Tinsel 2 however many lines are needed
	for (_ctx->sub = 0; _ctx->sub < (TinselV2 ? SubStringCount(hText) : 1); _ctx->sub++) {
		if (_ctx->myEscape && _ctx->myEscape != GetEscEvents())
			break;

		if (!_vm->_sound->sampleExists(hText))
			_ctx->bSample = false;
		else {
			// Kick off the voice sample
			_vm->_sound->playSample(hText, _ctx->sub, false, -1, -1, PRIORITY_TALK,
				Audio::Mixer::kSpeechSoundType, &_ctx->handle);
			_ctx->bSample = true;
		}

		// Display the text and set it's Z position
		if (event == POINTED || (!isJapanMode() && (_vm->_config->_useSubtitles || !_ctx->bSample))) {
			int	xshift;

			// Get the text string
			if (TinselV2)
				LoadSubString(hText, _ctx->sub, TextBufferAddr(), TBUFSZ);
			else
				LoadStringRes(hText, TextBufferAddr(), TBUFSZ);

			_ctx->pText = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), TextBufferAddr(),
						0, _ctx->textx, _ctx->texty, GetTagFontHandle(), TXT_CENTER);
			assert(_ctx->pText); // PrintObj() string produced NULL text

			MultiSetZPosition(_ctx->pText, Z_INV_ITEXT);

			if (TinselV2)
				KeepOnScreen(_ctx->pText, &_ctx->textx, &_ctx->texty);
			else {
				// Don't go off the side of the screen
				xshift = MultiLeftmost(_ctx->pText);
				if (xshift < 0) {
					MultiMoveRelXY(_ctx->pText, - xshift, 0);
					_ctx->textx -= xshift;
				}
				xshift = MultiRightmost(_ctx->pText);
				if (xshift > SCREEN_WIDTH) {
					MultiMoveRelXY(_ctx->pText, SCREEN_WIDTH - xshift, 0);
					_ctx->textx += SCREEN_WIDTH - xshift;
				}
			}
		} else
			_ctx->pText = NULL;

		if (TinselV2) {
			if (event == POINTED) {
				/*
				* PrintObj() called from POINT event
				*/
				// Have to give way to non-POINTED-generated text
				// and go away if the item gets picked up
				int x, y;
				do {
					// Give up if this item gets picked up
					if (WhichItemHeld() == pinvo->id)
						break;

					// Give way to non-POINTED-generated text
					if (g_bNotPointedRunning) {
						// Delete the text, and wait for the all-clear
						MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), _ctx->pText);
						_ctx->pText = NULL;

						while (g_bNotPointedRunning)
							CORO_SLEEP(1);

						GetCursorXY(&x, &y, false);
						if (InvItem(&x, &y, false) != _ctx->item)
							break;

						// Re-display in the same place
						LoadStringRes(hText, TextBufferAddr(), TBUFSZ);
						_ctx->pText = ObjectTextOut(GetPlayfieldList(FIELD_STATUS),
							TextBufferAddr(), 0, _ctx->textx, _ctx->texty, GetTagFontHandle(),
							TXT_CENTER, 0);
						assert(_ctx->pText);

						KeepOnScreen(_ctx->pText, &_ctx->textx, &_ctx->texty);
						MultiSetZPosition(_ctx->pText, Z_INV_ITEXT);
					}

					CORO_SLEEP(1);

					// Carry on until the cursor leaves this icon
					GetCursorXY(&x, &y, false);

				} while (InvItemId(x, y) == pinvo->id);
			} else {
				/*
				 * PrintObj() called from other event
				 */
				_ctx->myEscape = GetLeftEvents();
				_ctx->bTookControl = GetControl();

				// Display for a time, but abort if conversation gets hidden
				if (_ctx->pText)
					_ctx->ticks = TextTime(TextBufferAddr());
				_ctx->timeout = SAMPLETIMEOUT;

				for (;;) {
					CORO_SLEEP(1);

					// Abort if left click - hardwired feature for talky-print!
					// Abort if sample times out
					// Abort if conversation hidden
					if (LeftEventChange(_ctx->myEscape)
							|| --_ctx->timeout <= 0
							|| ConvIsHidden())
						break;

					if (_ctx->bSample) {
						// Wait for sample to end whether or not
						if (!_vm->_mixer->isSoundHandleActive(_ctx->handle)) {
							if (_ctx->pText == NULL || _vm->_config->_textSpeed == DEFTEXTSPEED) {
								// No text or speed modification - just depends on sample
								break;
							} else {
								// Must wait for time
								_ctx->bSample = false;
							}
						}

						// Decrement the subtitles timeout counter
						if (_ctx->ticks > 0) --_ctx->ticks;

					} else {
						// No sample - just depends on time
						if (_ctx->ticks-- <= 0)
							break;
					}
				}

				if (_ctx->bTookControl)
					ControlOn();		// Free control if we took it
			}

		} else {
			if (event == POINTED) {
				// FIXME: Is there ever an associated sound if in POINTED mode???
				assert(!_vm->_sound->sampleExists(hText));
				CORO_INVOKE_ARGS(PrintObjPointed, (CORO_SUBCTX, hText, pinvo, _ctx->pText, _ctx->textx, _ctx->texty, _ctx->item));
			} else {
				CORO_INVOKE_2(PrintObjNonPointed, hText, _ctx->pText);
			}
		}

		// Delete the text, if haven't already
		if (_ctx->pText)
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), _ctx->pText);

		// If it hasn't already finished, stop sample
		if (_ctx->bSample)
			_vm->_mixer->stopHandle(_ctx->handle);
	}

	// Let POINTED text back in if this is the last
	if (event != POINTED)
		g_bNotPointedRunning = false;

	CORO_END_CODE;
}

static void PrintObjPointed(CORO_PARAM, const SCNHANDLE text, const INV_OBJECT *pinvo, OBJECT *&pText, const int textx, const int texty, const int item) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
		// Have to give way to non-POINTED-generated text
		// and go away if the item gets picked up
		int	x, y;
		do {
			// Give up if this item gets picked up
			if (WhichItemHeld() == pinvo->id)
				break;

			// Give way to non-POINTED-generated text
			if (g_bNotPointedRunning) {
				// Delete the text, and wait for the all-clear
				MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), pText);
				pText = NULL;
				while (g_bNotPointedRunning)
					CORO_SLEEP(1);

				GetCursorXY(&x, &y, false);
				if (InvItem(&x, &y, false) != item)
					break;

				// Re-display in the same place
				LoadStringRes(text, TextBufferAddr(), TBUFSZ);
				pText = ObjectTextOut(GetPlayfieldList(FIELD_STATUS), TextBufferAddr(),
							0, textx, texty, GetTagFontHandle(), TXT_CENTER);
				assert(pText); // PrintObj() string produced NULL text
				MultiSetZPosition(pText, Z_INV_ITEXT);
			}

			CORO_SLEEP(1);

			// Carry on until the cursor leaves this icon
			GetCursorXY(&x, &y, false);
		} while (InvItemId(x, y) == pinvo->id);

	CORO_END_CODE;
}

static void PrintObjNonPointed(CORO_PARAM, const SCNHANDLE text, const OBJECT *pText) {
	CORO_BEGIN_CONTEXT;
		bool bSample;		// Set if a sample is playing
		Audio::SoundHandle handle;

		int myleftEvent;
		bool took_control;
		int	ticks;
		int	timeout;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
		// Kick off the voice sample
		if (_vm->_config->_voiceVolume != 0 && _vm->_sound->sampleExists(text)) {
			_vm->_sound->playSample(text, Audio::Mixer::kSpeechSoundType, &_ctx->handle);
			_ctx->bSample = _vm->_mixer->isSoundHandleActive(_ctx->handle);
		} else
			_ctx->bSample = false;

		_ctx->myleftEvent = GetLeftEvents();
		_ctx->took_control = GetControl(CONTROL_OFF);

		// Display for a time, but abort if conversation gets hidden
		if (isJapanMode())
			_ctx->ticks = JAP_TEXT_TIME;
		else if (pText)
			_ctx->ticks = TextTime(TextBufferAddr());
		else
			_ctx->ticks = 0;

		_ctx->timeout = SAMPLETIMEOUT;
		do {
			CORO_SLEEP(1);
			--_ctx->timeout;

			// Abort if left click - hardwired feature for talky-print!
			// Abort if sample times out
			// Abort if conversation hidden
			if (_ctx->myleftEvent != GetLeftEvents() || _ctx->timeout <= 0 || ConvIsHidden())
				break;

			if (_ctx->bSample) {
				// Wait for sample to end whether or not
				if (!_vm->_mixer->isSoundHandleActive(_ctx->handle)) {
					if (pText == NULL || _vm->_config->_textSpeed == DEFTEXTSPEED) {
						// No text or speed modification - just depends on sample
						break;
					} else {
						// Must wait for time
						_ctx->bSample = false;
					}
				}

				// Decrement the subtitles timeout counter
				if (_ctx->ticks > 0) --_ctx->ticks;

			} else {
				// No sample - just depends on time
				if (_ctx->ticks-- <= 0)
					break;
			}
		} while (1);

		g_bNotPointedRunning = false;	// Let POINTED text back in

		if (_ctx->took_control)
			Control(CONTROL_ON);	// Free control if we took it

		_vm->_mixer->stopHandle(_ctx->handle);

	CORO_END_CODE;
}

/**
 * Register the fact that this poly would like its tag displayed.
 */
static void PrintTag(HPOLYGON hp, SCNHANDLE text, int actor = 0, bool bCursor = false) {
	// printtag() may only be called from a polygon code block in Tinsel 1, or
	// additionally from a moving actor code block in Tinsel 2
	assert((hp != NOPOLY) || (TinselV2 && (actor != 0)));

	if (hp != NOPOLY) {
		// Poly handling
		if (TinselV2)
			SetPolyTagWanted(hp, true, bCursor, text);
		else if (PolyTagState(hp) == TAG_OFF) {
			SetPolyTagState(hp, TAG_ON);
			SetPolyTagHandle(hp, text);
		}
	} else {
		// Moving actor handling
		SetActorTagWanted(actor, true, bCursor, text);
	}
}

/**
 * Quits the game
 */
static void QuitGame() {
	StopMidi();
	StopSample();
	_vm->quitGame();
}

/**
 * Return a random number between optional limits.
 */
static int RandomFn(int n1, int n2, int norpt) {
	int i = 0;
	uint32 value;

	// In DW1 demo, upper/lower limit can be reversed
	if (n2 < n1) SWAP(n1, n2);

	do {
		value = n1 + _vm->getRandomNumber(n2 - n1);
	} while ((g_lastValue == value) && (norpt == RAND_NORPT) && (++i <= 10));

	g_lastValue = value;
	return value;
}

/**
 * ResetIdleTime
 */
void ResetIdleTime() {
	resetUserEventTime();
}

/**
 * FnRestartGame
 */
void FnRestartGame() {
	// TODO: Tinsel 2 comments out the 2 calls, but I'm not sure that this should be done
	StopMidi();
	StopSample();

	g_bRestart = true;
	g_sceneCtr = 0;
}

/**
 * Restore saved scene.
 */
static void RestoreScene(CORO_PARAM, TRANSITS transition) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (TinselV2) {
		if (_vm->_bmv->MoviePlaying()) {
			_vm->_bmv->AbortMovie();
			CORO_SLEEP(2);
		}

		CuttingScene(false);

	} else {
		UnSuspendHook();
	}

	TinselRestoreScene(transition == TRANS_FADE);

	CORO_END_CODE;
}

/**
 * Resumes the last game
 */
void ResumeLastGame() {
	RestoreGame(NewestSavedGame());
}

/**
 * Returns the current run mode
 */
static int RunMode() {
	return 0;	//clRunMode;
}

/**
 * SamplePlaying
 */
static bool SamplePlaying(bool escOn, int myEscape) {
	// escape effects introduced 14/12/95 to fix
	//	 while (sampleplaying()) pause;

	if (escOn && myEscape != GetEscEvents())
		return false;

	return _vm->_sound->sampleIsPlaying();
}

/**
 * Save current scene.
 */
void SaveScene(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (TinselV2) {
		CuttingScene(true);
		SendSceneTinselProcess(LEAVE_T2);
		CORO_GIVE_WAY;

		CORO_INVOKE_0(TinselSaveScene);
	} else {
		CORO_INVOKE_0(TinselSaveScene);
		SuspendHook();
	}

	CORO_END_CODE;
}

/**
 * ScalingReels
 */
static void ScalingReels(int actor, int scale, int direction,
		SCNHANDLE left, SCNHANDLE right, SCNHANDLE forward, SCNHANDLE away) {

	SetScalingReels(actor, scale, direction, left, right, forward, away);
}

/**
 * Return the icon that caused the CONVERSE event.
 */
static int ScanIcon() {
	return GetIcon();
}

/**
 * Scroll the screen to target co-ordinates.
 */
static void Scroll(CORO_PARAM, EXTREME extreme, int xp, int yp, int xIter, int yIter, bool bComp, bool escOn, int myEscape) {
	CORO_BEGIN_CONTEXT;
		int	thisScroll;
		int x, y;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	++g_scrollNumber;
	_ctx->x = xp;
	_ctx->y = yp;

	if ((TinselV2 && g_bInstantScroll) || (escOn && myEscape != GetEscEvents())) {
		// Instant completion!
		Offset(extreme, _ctx->x, _ctx->y);
	} else {
		_ctx->thisScroll = g_scrollNumber;
		if (TinselV2)
			DecodeExtreme(extreme, &_ctx->x, &_ctx->y);

		ScrollTo(_ctx->x, _ctx->y, xIter, yIter);

		if (bComp) {
			int	Loffset, Toffset;
			do {
				CORO_SLEEP(1);

				// If escapable and ESCAPE is pressed...
				if (escOn && myEscape != GetEscEvents()) {
					// Instant completion!
					Offset(extreme, _ctx->x, _ctx->y);
					break;
				}

				// give up if have been superseded
				if (_ctx->thisScroll != g_scrollNumber)
					CORO_KILL_SELF();

				PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);
			} while (Loffset != _ctx->x || Toffset != _ctx->y);
		} else if (TinselV2 && myEscape) {
			SCROLL_MONITOR sm;

			// Scroll is escapable even though we're not waiting for it
			sm.x = _ctx->x;
			sm.y = _ctx->y;
			sm.thisScroll = g_scrollNumber;
			sm.myEscape = myEscape;
			CoroScheduler.createProcess(PID_TCODE, ScrollMonitorProcess, &sm, sizeof(sm));
		}
	}
	CORO_END_CODE;
}

/**
 * ScrollParameters
 */
static void ScrollParameters(int xTrigger, int xDistance, int xSpeed, int yTriggerTop,
		int yTriggerBottom, int yDistance, int ySpeed) {
	SetScrollParameters(xTrigger, xDistance, xSpeed,
			yTriggerTop, yTriggerBottom, yDistance, ySpeed);
}

/**
 * SendActor("actor", event)
 */
int SendActor(CORO_PARAM, int actor, TINSEL_EVENT event, HPOLYGON hp, int myEscape) {
	bool result;

	if (IsTaggedActor(actor)) {
		assert(actor);
		ActorEvent(coroParam, actor, event, true, myEscape, &result);
	} else {
		SendTag(coroParam, actor | ACTORTAG_KEY, event, hp, myEscape, &result);
	}

	return result;
}

/**
 * SendGlobalProcess(process#, event)
 */
static int SendGlobalProcess(CORO_PARAM, int procId, TINSEL_EVENT event, int myEscape) {
	return GlobalProcessEvent(coroParam, procId, event, true, myEscape);
}

/**
 * SendObject(object, event)
 */
static int SendObject(CORO_PARAM, int object, TINSEL_EVENT event, int myEscape) {
	bool result;
	ObjectEvent(coroParam, object, event, true, myEscape, &result);
	return result;
}

/**
 * SendProcess(process#, event)
 */
static int SendProcess(CORO_PARAM, int procId, TINSEL_EVENT event, int myEscape) {
	bool result;
	SceneProcessEvent(coroParam, procId, event, true, myEscape, &result);
	return result;
}

/**
 * SendTag(tag#, event)
 */
static void SendTag(CORO_PARAM, int tagno, TINSEL_EVENT event, HPOLYGON hp, int myEscape, bool *result) {
	// Tag could be zero, meaning calling tag
	if (tagno == 0) {
		assert(hp != NOPOLY);

		PolygonEvent(coroParam, hp, event, 0, true, myEscape, result);
	} else {
		assert(IsTagPolygon(tagno));

		PolygonEvent(coroParam, GetTagHandle(tagno), event, 0, true, myEscape, result);
	}
}

/**
 * Un-kill an actor.
 */
static void SetActor(int actor) {
	EnableActor(actor);
}

/**
 * Turn a blocking polygon on.
 */

static void SetBlock(int blockno) {
	EnableBlock(blockno);
}

/**
 * Turn an exit on.
 */

static void SetExit(int exitno) {
	EnableExit(exitno);
}

/**
 * Guess what.
 */
static void SetInvLimit(int invno, int n) {
	InvSetLimit(invno, n);
}

/**
 * Guess what.
 */
static void SetInvSize(int invno, int MinWidth, int MinHeight,
		int StartWidth, int StartHeight, int MaxWidth, int MaxHeight) {
	InvSetSize(invno, MinWidth, MinHeight, StartWidth, StartHeight, MaxWidth, MaxHeight);
}

/**
 * Guess what.
 */
static void SetLanguage(LANGUAGE lang) {
	assert(lang == TXT_ENGLISH || lang == TXT_FRENCH
	     || lang == TXT_GERMAN  || lang == TXT_ITALIAN
	     || lang == TXT_SPANISH); // ensure language is valid

	ChangeLanguage(lang);
}

/**
 * Set palette
 */
static void SetPalette(SCNHANDLE hPal, bool escOn, int myEscape) {
	// Don't do it if it's not wanted
	if (escOn && myEscape != GetEscEvents())
		return;

	ChangePalette(hPal);
}

/**
 * SetSystemString
 */

static void SetSystemString(int stringId, SCNHANDLE hString) {
	SetSysString(stringId, hString);
}

/**
 * Set a system variable
 */
static void SetSystemVar(int varId, int newValue) {
	SetSysVar(varId, newValue);
}

/**
 * Turn a tag on.
 */
static void SetTag(CORO_PARAM, int tagno) {
	EnableTag(coroParam, tagno);
}

/**
 * Initialize a timer.
 */
static void SetTimer(int timerno, int start, bool up, bool frame) {
	StartTimer(timerno, start, up != 0, frame != 0);
}

/**
 * Shell("cmdline")
 */
static void Shell(SCNHANDLE commandLine) {
	LoadStringRes(commandLine, TextBufferAddr(), TBUFSZ);
	error("Tried to execute shell command \"%s\"", TextBufferAddr());
}

/**
 * Don't hide an actors graphics.
 */
static void ShowActorFn(CORO_PARAM, int actor) {
	ShowActor(coroParam, actor);
}

/**
 * Turn a blocking polygon on.
 */
void ShowBlock(int blockno) {
	EnableBlock(blockno);
}

/**
 * Turn an effect polygon on.
 */
void ShowEffect(int effect) {
	EnableEffect(effect);
}

#ifdef DEBUG
/**
 * Enable display of diagnostic co-ordinates.
 */
static void showpos() {
	setshowpos();
}

/**
 * Enable display of diagnostic co-ordinates.
 */
static void showstring() {
	setshowstring();
}
#endif

/**
 * Shows the main menu
 */
static void ShowMenu() {
	OpenMenu(MAIN_MENU);
}

/**
 * Turn a path on.
 */
static void ShowPath(int path) {
	EnablePath(path);
}

/**
 * Turn a refer on.
 */
void ShowRefer(int refer) {
	EnableRefer(refer);
}

/**
 * Turn a tag on.
 */
static void ShowTag(CORO_PARAM, int tag, HPOLYGON hp) {
	// Tag could be zero, meaning calling tag
	EnableTag(coroParam, tag ? tag : GetTagPolyId(hp));
}

/**
 * Special play - slow down associated actor's movement while the play
 * is running. After the play, position the actor where the play left
 * it and continue walking, if the actor still is.
 */
static void SPlay(CORO_PARAM, int sf, SCNHANDLE film, int x, int y, bool complete, int actorid, bool escOn, int myEscape) {
	// Don't do it if it's not wanted
	if (escOn && myEscape != GetEscEvents())
		return;

	Play(coroParam, film, x, y, complete, actorid, true, sf, escOn, myEscape, false);
}

/**
 * (Re)Position an actor.
 * If moving actor is not around yet in this scene, start it up.
 */
void Stand(CORO_PARAM, int actor, int x, int y, SCNHANDLE hFilm) {
	CORO_BEGIN_CONTEXT;
		PMOVER pMover;		// Moving actor structure
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->pMover = GetMover(actor);
	assert(!TinselV2 || (_ctx->pMover != NULL));

	if (_ctx->pMover) {
		if (TinselV2) {
			// New special. If no paths, just ignore this
			if (PathCount() == 0)
				return;

			// Another new special.
			// If lead actor, and TalkVia, ignore
			if ((actor == GetLeadId() || actor == LEAD_ACTOR) && SysVar(ISV_DIVERT_ACTOR))
				return;
		}

		if (!MoverIs(_ctx->pMover)) {
			// create a moving actor process
			MoverProcessCreate(x, y, (actor == LEAD_ACTOR) ? GetLeadId() : actor, _ctx->pMover);

			if (hFilm == TF_NONE) {
				// Make sure there is an assigned actorObj
				while (!_ctx->pMover->actorObj)
					CORO_SLEEP(1);

				SetMoverStanding(_ctx->pMover);
			} else {
				// Check hFilm against certain constants. Note that a switch statement isn't
				// used here because it would interfere with our co-routine implementation
				if (hFilm == TF_UP) {
					if (TinselV2) CORO_GIVE_WAY;
					SetMoverDirection(_ctx->pMover, AWAY);
					SetMoverStanding(_ctx->pMover);
				} else if (hFilm == TF_DOWN) {
					if (TinselV2) CORO_GIVE_WAY;
					SetMoverDirection(_ctx->pMover, FORWARD);
					SetMoverStanding(_ctx->pMover);
				} else if (hFilm == TF_LEFT) {
					if (TinselV2) CORO_GIVE_WAY;
					SetMoverDirection(_ctx->pMover, LEFTREEL);
					SetMoverStanding(_ctx->pMover);
				} else if (hFilm == TF_RIGHT) {
					if (TinselV2) CORO_GIVE_WAY;
					SetMoverDirection(_ctx->pMover, RIGHTREEL);
					SetMoverStanding(_ctx->pMover);
				} else if (hFilm != TF_NONE) {
					if (TinselV2) CORO_GIVE_WAY;
					AlterMover(_ctx->pMover, hFilm, AR_NORMAL);
				}
			}
		} else {
			switch (hFilm) {
			case TF_NONE:
				if (x != -1 && y != -1)
					PositionMover(_ctx->pMover, x, y);
				break;

			case TF_UP:
				SetMoverDirection(_ctx->pMover, AWAY);
				if (x != -1 && y != -1)
					PositionMover(_ctx->pMover, x, y);
				SetMoverStanding(_ctx->pMover);
				break;
			case TF_DOWN:
				SetMoverDirection(_ctx->pMover, FORWARD);
				if (x != -1 && y != -1)
					PositionMover(_ctx->pMover, x, y);
				SetMoverStanding(_ctx->pMover);
				break;
			case TF_LEFT:
				SetMoverDirection(_ctx->pMover, LEFTREEL);
				if (x != -1 && y != -1)
					PositionMover(_ctx->pMover, x, y);
				SetMoverStanding(_ctx->pMover);
				break;
			case TF_RIGHT:
				SetMoverDirection(_ctx->pMover, RIGHTREEL);
				if (x != -1 && y != -1)
					PositionMover(_ctx->pMover, x, y);
				SetMoverStanding(_ctx->pMover);
				break;

			default:
				if (x != -1 && y != -1)
					PositionMover(_ctx->pMover, x, y);
				AlterMover(_ctx->pMover, hFilm, AR_NORMAL);
				break;
			}
		}
	} else if (actor == NULL_ACTOR) {
		//
	} else {
		assert(hFilm != 0); // Trying to play NULL film

		// Kick off the play and return.
		CORO_INVOKE_ARGS(PlayFilm, (CORO_SUBCTX, hFilm, x, y, actor, false, 0, false, 0, false));
	}

	CORO_END_CODE;
}

/**
 * Position the actor at the polygon's tag node.
 */
static void StandTag(int actor, HPOLYGON hp) {
	SCNHANDLE hFilm;
	int	pnodex, pnodey;

	assert(hp != NOPOLY); // StandTag() may only be called from a polygon code block

	// Where to stand
	GetPolyNode(hp, &pnodex, &pnodey);

	// Lead actor uses tag node film
	hFilm = GetPolyFilm(hp);

	// other actors can use direction
	if (TinselV2) {
		if (actor != LEAD_ACTOR && actor != GetLeadId()
				&& hFilm != TF_UP && hFilm != TF_DOWN
				&& hFilm != TF_LEFT && hFilm != TF_RIGHT)
			hFilm = 0;

		Stand(Common::nullContext, actor, pnodex, pnodey, hFilm);

	} else if (hFilm && (actor == LEAD_ACTOR || actor == GetLeadId()))
		Stand(Common::nullContext, actor, pnodex, pnodey, hFilm);
	else
		Stand(Common::nullContext, actor, pnodex, pnodey, 0);
}


/**
 * StartGlobalProcess
 */
static void StartGlobalProcess(CORO_PARAM, uint32 procID) {
	GlobalProcessEvent(coroParam, procID, STARTUP, false, 0);
}

/**
 * StartProcess
 */
static void StartProcess(CORO_PARAM, uint32 procID) {
	SceneProcessEvent(coroParam, procID, STARTUP, false, 0);
}

/**
 * Initialize a timer.
 */
static void StartTimerFn(int timerno, int start, bool up, int fs) {
	StartTimer(timerno, start, up, fs);
}

void StopMidiFn() {
	StopMidi();		// Stop any currently playing midi
}

/**
 * Kill a specific sample, or all samples.
 */
void StopSample(int sample) {
	if (sample == -1)
		_vm->_sound->stopAllSamples();		// Stop any currently playing sample
	else
		_vm->_sound->stopSpecSample(sample, 0);
}

/**
 * Kill a moving actor's walk.
 */
static void StopWalk(int actor) {
	PMOVER pMover;

	pMover = GetMover(actor);
	assert(pMover);

	if (TinselV2) {
		if (MoverHidden(pMover))
			return;

		StopMover(pMover);		// Cause the actor to stop
	} else {
		GetToken(pMover->actorToken);	// Kill the walk process
		pMover->bStop = true;			// Cause the actor to stop
		FreeToken(pMover->actorToken);
	}
}

/**
 * Subtitles on/off
 */
static void Subtitles(int onoff) {
	assert (onoff == ST_ON || onoff == ST_OFF);

	if (isJapanMode())
		return;	// Subtitles are always off in JAPAN version (?)

	_vm->_config->_useSubtitles = (onoff == ST_ON);
}

/**
 * Special walk.
 * Walk into or out of a legal path.
 */
static void Swalk(CORO_PARAM, int actor, int x1, int y1, int x2, int y2, SCNHANDLE film, int32 zOverride, bool escOn, int myEscape) {
	CORO_BEGIN_CONTEXT;
		bool	bTookControl;			// Set if this function takes control
	CORO_END_CONTEXT(_ctx);

	HPOLYGON hPath;

	CORO_BEGIN_CODE(_ctx);

	// Don't do it if it's not wanted
	if (escOn && myEscape != GetEscEvents()) {
		if (TinselV2) {
			if (x2 == -1 && y2 == -1)
				CORO_INVOKE_ARGS(Stand, (CORO_SUBCTX, actor, x1, y1, 0));
			else
				CORO_INVOKE_ARGS(Stand, (CORO_SUBCTX, actor, x2, y2, 0));
		}

		return;
	}

	// For lead actor, lock out the user (if not already locked out)
	if (actor == GetLeadId() || actor == LEAD_ACTOR) {
		_ctx->bTookControl = GetControl(CONTROL_OFFV2);
		if (TinselV2 && _ctx->bTookControl)
			RestoreMainCursor();
	} else {
		_ctx->bTookControl = false;
	}

	if (TinselV2 && (x2 == -1) && (y2 == -1)) {
		// First co-ordinates are the destination
		x2 = x1;
		y2 = y1;
	} else {
		// Stand at the start co-ordinates
		hPath = InPolygon(x1, y1, PATH);

		if (hPath != NOPOLY) {
			// Walking out of a path
			CORO_INVOKE_ARGS(Stand, (CORO_SUBCTX, actor, x1, y1, 0));
		} else {
			hPath = InPolygon(x2, y2, PATH);
			// One of them has to be in a path
			assert(hPath != NOPOLY); //one co-ordinate must be in a legal path

			// Walking into a path
			CORO_INVOKE_ARGS(Stand, (CORO_SUBCTX, actor, x2, y2, 0));	// Get path's characteristics
			CORO_INVOKE_ARGS(Stand, (CORO_SUBCTX, actor, x1, y1, 0));
		}

		if (TinselV2 && (zOverride != -1)) {
			PMOVER pMover = GetMover(actor);
			assert(pMover);

			SetMoverZ(pMover, y1, zOverride);
		}
	}

	CORO_INVOKE_ARGS(Walk, (CORO_SUBCTX, actor, x2, y2, film, 0, true, zOverride, escOn, myEscape));

	// Free control if we took it
	if (_ctx->bTookControl)
		Control(CONTROL_ON);

	CORO_END_CODE;
}

/**
 * Gets a system variable
 */
static int SystemVar(int varId) {
	return SysVar(varId);
}

/**
 * Define a tagged actor.
 */
static void TagActor(int actor, SCNHANDLE text, int tp) {
	Tag_Actor(actor, text, tp);
}

/**
 * TagPos([tag #])
 */
static int TagPos(MASTER_LIB_CODES operand, int tagno, HPOLYGON hp) {
	int	x, y;

	// Tag could be zero, meaning calling tag
	if (tagno == 0)
		tagno = GetTagPolyId(hp);

	if (operand == TAGTAGXPOS || operand == TAGTAGYPOS) {
		SCNHANDLE junk;

		GetTagTag(GetTagHandle(tagno), &junk, &x, &y);
	} else {
		GetPolyNode(GetTagHandle(tagno), &x, &y);
	}

	if (operand == TAGTAGXPOS || operand == TAGWALKXPOS)
		return x;
	else
		return y;
}

/**
 * Text goes over actor's head while actor plays the talk reel.
 */
static void FinishTalkingReel(CORO_PARAM, PMOVER pMover, int actor) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (pMover) {
		SetMoverStanding(pMover);
		AlterMover(pMover, 0, AR_POPREEL);
	} else {
		SetActorTalking(actor, false);
		CORO_INVOKE_ARGS(PlayFilm, (CORO_SUBCTX, GetActorPlayFilm(actor), -1, -1, 0, false, 0, false, 0, false));
	}

	CORO_END_CODE;
}

static void TalkOrSay(CORO_PARAM, SPEECH_TYPE speechType, SCNHANDLE hText, int x, int y,
					  SCNHANDLE hFilm, int actorId, bool bSustain, bool escOn, int myEscape) {
	CORO_BEGIN_CONTEXT;
		int		Loffset, Toffset;	// Top left of display
		int		actor;			// The speaking actor
		PMOVER	pActor;			// For moving actors
		int		myLeftEvent;
		int		escEvents;
		int		ticks;
		bool	bTookControl;	// Set if this function takes control
		bool	bTookTags;		// Set if this function disables tags
		OBJECT	*pText;			// text object pointer
		bool	bSample;		// Set if a sample is playing
		bool	bSamples;
		bool	bTalkReel;		// Set while talk reel is playing
		Audio::SoundHandle handle;
		int	timeout;

		SPEECH_TYPE whatSort;
		TFTYPE	direction;
		int sub;
		int x, y;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->whatSort = speechType;
	_ctx->escEvents = myEscape;
	_ctx->x = x;
	_ctx->y = y;
	_ctx->Loffset = 0;
	_ctx->Toffset = 0;
	_ctx->ticks = 0;
	_ctx->pText = NULL;

	// If waiting is enabled, wait for ongoing scroll
	if (TinselV2 && SysVar(SV_SPEECHWAITS))
		CORO_INVOKE_1(WaitScroll, myEscape);

	// Don't do it if it's not wanted
	if (escOn && myEscape != GetEscEvents())
		return;

	_ctx->myLeftEvent = GetLeftEvents();

	// If this actor is dead, call a stop to the calling process
	if (!TinselV2 && (actorId && !actorAlive(actorId)))
		CORO_KILL_SELF();

	if (!TinselV2 || (speechType == IS_TALK)) {
		/*
		 * Find out which actor is talking
		 * and with which direction if no film supplied
		 */
		switch (hFilm) {
		case TF_NONE:
		case TF_UP:
		case TF_DOWN:
		case TF_LEFT:
		case TF_RIGHT:
			_ctx->actor = GetLeadId();	// If no film, actor is lead actor
			_ctx->direction = (TFTYPE)hFilm;
			break;

		default:
			_ctx->actor = ExtractActor(hFilm);
			assert(_ctx->actor); // talk() - no actor ID in the reel
			_ctx->direction = TF_FILM;
			break;
		}
		assert(_ctx->actor);
	} else if (TinselV2)
		_ctx->actor = actorId;

	/*
	 * Lock out the user (for lead actor, if not already locked out)
	 * May need to disable tags for other actors
	 */
	if (_ctx->actor == GetLeadId() || (TinselV2 && (_ctx->actor == LEAD_ACTOR)))
		_ctx->bTookControl = GetControl(CONTROL_OFF);
	else
		_ctx->bTookControl = false;
	_ctx->bTookTags = DisableTagsIfEnabled();

	if (TinselV2) {
		/*
		 * Divert stuff
		 */
		if (SysVar(ISV_DIVERT_ACTOR) && (_ctx->actor == GetLeadId() || _ctx->actor == LEAD_ACTOR)) {
			_ctx->actor = SysVar(ISV_DIVERT_ACTOR);
			if (_ctx->whatSort == IS_TALK)
				_ctx->whatSort = IS_SAY;
			else if (_ctx->whatSort == IS_TALKAT)
				_ctx->whatSort = IS_SAYAT;
		}
	}

	/*
	 * Kick off the voice sample
	 */
	if (_vm->_config->_voiceVolume != 0 && _vm->_sound->sampleExists(hText)) {
		if (!TinselV2) {
			_vm->_sound->playSample(hText, Audio::Mixer::kSpeechSoundType, &_ctx->handle);
			_ctx->bSamples = _vm->_mixer->isSoundHandleActive(_ctx->handle);
		} else
			_ctx->bSamples = true;
	} else
		_ctx->bSamples = false;

	/*
	 * Replace actor with the talk reel, saving the current one
	 */
	_ctx->pActor = GetMover(_ctx->actor);
	if (_ctx->whatSort == IS_TALK) {
		if (_ctx->pActor) {
			if (_ctx->direction != TF_FILM)
				hFilm = GetMoverTalkReel(_ctx->pActor, _ctx->direction);
			AlterMover(_ctx->pActor, hFilm, AR_PUSHREEL);
		} else {
			SetActorTalking(_ctx->actor, true);
			SetActorTalkFilm(_ctx->actor, hFilm);
			CORO_INVOKE_ARGS(PlayFilm, (CORO_SUBCTX, hFilm, -1, -1, 0, false, 0, escOn, myEscape, false));
		}
		_ctx->bTalkReel = true;
		CORO_SLEEP(1);		// Allow the play to come in

	} else if (_ctx->whatSort == IS_TALKAT) {
		_ctx->bTalkReel = false;

	} else if ((_ctx->whatSort == IS_SAY) || (_ctx->whatSort == IS_SAYAT)) {
		_ctx->bTalkReel = false;
		if (IsTaggedActor(_ctx->actor)) {
			CORO_INVOKE_ARGS(ActorEvent, (CORO_SUBCTX, _ctx->actor, TALKING, false, 0));
		} else if (IsTagPolygon(_ctx->actor | ACTORTAG_KEY)) {
			CORO_INVOKE_ARGS(PolygonEvent, (CORO_SUBCTX, GetTagHandle(_ctx->actor | ACTORTAG_KEY),
				TALKING, 0, false, 0));
		}

		if (TinselV2)
			// Let it all kick in and position this 'waiting' process
			// down the process list from the playing process(es)
			// This ensures immediate return when the reel finishes
			CORO_GIVE_WAY;
	}

	// Make multi-ones escape
	if (TinselV2 && (SubStringCount(hText) > 1) && !_ctx->escEvents)
		_ctx->escEvents = GetEscEvents();

	for (_ctx->sub = 0; _ctx->sub < (TinselV2 ? SubStringCount(hText) : 1); _ctx->sub++) {
		if (TinselV2 && _ctx->escEvents && _ctx->escEvents != GetEscEvents())
			break;

		/*
		 * Display the text.
		 */
		_ctx->bSample = _ctx->bSamples;
		_ctx->pText = NULL;

		if (isJapanMode()) {
			_ctx->ticks = JAP_TEXT_TIME;
		} else if (_vm->_config->_useSubtitles || !_ctx->bSample) {
			/*
			 * Work out where to display the text
			 */
			int	xshift, yshift;

			PlayfieldGetPos(FIELD_WORLD, &_ctx->Loffset, &_ctx->Toffset);
			if ((_ctx->whatSort == IS_SAY) || (_ctx->whatSort == IS_TALK))
				GetActorMidTop(_ctx->actor, &_ctx->x, &_ctx->y);

			if (!TinselV0)
				SetTextPal(GetActorRGB(_ctx->actor));
			if (TinselV2)
				LoadSubString(hText, _ctx->sub, TextBufferAddr(), TBUFSZ);
			else {
				LoadStringRes(hText, TextBufferAddr(), TBUFSZ);

				_ctx->y -= _ctx->Toffset;
			}

			_ctx->pText = ObjectTextOut(GetPlayfieldList(FIELD_STATUS),
					TextBufferAddr(), 0, _ctx->x - _ctx->Loffset, _ctx->y - _ctx->Toffset,
					GetTalkFontHandle(), TXT_CENTER);
			assert(_ctx->pText); // talk() string produced NULL text;

			if (IsTopWindow())
				MultiSetZPosition(_ctx->pText, Z_TOPW_TEXT);

			if ((_ctx->whatSort == IS_SAY) || (_ctx->whatSort == IS_TALK)) {
				/*
				 * Set bottom of text just above the speaker's head
				 * But don't go off the top of the screen
				 */
				if (TinselV2)
					MultiMoveRelXY(_ctx->pText, 0, _ctx->y - _ctx->Toffset - MultiLowest(_ctx->pText) - 2);
				else {
					yshift = _ctx->y - MultiLowest(_ctx->pText) - 2;		// Just above head
					MultiMoveRelXY(_ctx->pText, 0, yshift);		//
					yshift = MultiHighest(_ctx->pText);
					if (yshift < 4)
						MultiMoveRelXY(_ctx->pText, 0, 4 - yshift);	// Not off top

					/*
					 * Don't go off the side of the screen
					 */
					xshift = MultiRightmost(_ctx->pText) + 2;
					if (xshift >= SCREEN_WIDTH)			// Not off right
						MultiMoveRelXY(_ctx->pText, SCREEN_WIDTH - xshift, 0);
					xshift = MultiLeftmost(_ctx->pText) - 1;
					if (xshift <= 0)					// Not off left
						MultiMoveRelXY(_ctx->pText, -xshift, 0);
				}
			}

			if (TinselV2)
				// Don't go off the screen
				KeepOnScreen(_ctx->pText, &_ctx->x, &_ctx->y);

			/*
			 * Work out how long to talk.
			 * During this time, reposition the text if the screen scrolls.
			 */
			_ctx->ticks = TextTime(TextBufferAddr());
		}

		if (TinselV2 && _ctx->bSample) {
			// Kick off the sample now (perhaps with a delay)
			if (g_bNoPause)
				g_bNoPause = false;
			else if (!TinselV2Demo)
				CORO_SLEEP(SysVar(SV_SPEECHDELAY));

			//SamplePlay(VOICE, hText, _ctx->sub, false, -1, -1, PRIORITY_TALK);
			_vm->_sound->playSample(hText, _ctx->sub, false, -1, -1, PRIORITY_TALK, Audio::Mixer::kSpeechSoundType, &_ctx->handle);
		}

		_ctx->timeout = SAMPLETIMEOUT;

		do {
			// Keep text in place if scrolling
			if (_ctx->pText != NULL) {
				int	nLoff, nToff;

				PlayfieldGetPos(FIELD_WORLD, &nLoff, &nToff);
				if (nLoff != _ctx->Loffset || nToff != _ctx->Toffset) {
					MultiMoveRelXY(_ctx->pText, _ctx->Loffset - nLoff, _ctx->Toffset - nToff);
					_ctx->Loffset = nLoff;
					_ctx->Toffset = nToff;
				}
			}

			CORO_SLEEP(1);

			// Handle timeout decrementing and Escape presses
			if (TinselV2) {
				if ((_ctx->escEvents && _ctx->escEvents != GetEscEvents()) ||
					(!bSustain && LeftEventChange(_ctx->myLeftEvent)) ||
					(--_ctx->timeout <= 0)) {
					// Left event only kills current sub-string
					_ctx->myLeftEvent = GetLeftEvents();
					break;
				}
			} else {
				--_ctx->timeout;

				// Abort if escapable and ESCAPE is pressed
				// Abort if left click - hardwired feature for talk!
				// Abort if sample times out
				if ((escOn && myEscape != GetEscEvents())
						|| (_ctx->myLeftEvent != GetLeftEvents())
						|| (_ctx->timeout <= 0))
					break;
			}

			if (_ctx->bSample) {
				// Wait for sample to end whether or not
				if (!_vm->_mixer->isSoundHandleActive(_ctx->handle)) {
					if (_ctx->pText == NULL || _vm->_config->_textSpeed == DEFTEXTSPEED) {
						// No text or speed modification - just depends on sample
						break;
					} else {
						// Talk reel stops at end of speech
						if (!TinselV2 || (_ctx->bTalkReel && (_ctx->sub == SubStringCount(hText) - 1))) {
							CORO_INVOKE_2(FinishTalkingReel, _ctx->pActor, _ctx->actor);
							_ctx->bTalkReel = false;
						}
						_ctx->bSample = false;
					}
				}

				// Decrement the subtitles timeout counter
				if (_ctx->ticks > 0) --_ctx->ticks;

			} else {
				// No sample - just depends on time
				if (_ctx->ticks-- <= 0)
					break;
			}
		} while (1);

		if (_ctx->pText != NULL) {
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), _ctx->pText);
			_ctx->pText = NULL;
		}
		if (TinselV2 && _ctx->bSample)
			_vm->_sound->stopSpecSample(hText, _ctx->sub);
	}

	/*
	 * The talk is over now - dump the text
	 * Stop the sample
	 * Restore the actor's film or standing reel
	 */
	if (_ctx->bTalkReel)
		CORO_INVOKE_2(FinishTalkingReel, _ctx->pActor, _ctx->actor);
	if (_ctx->pText != NULL)
		MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), _ctx->pText);

	if (TinselV2) {
		if ((_ctx->whatSort == IS_SAY) || (_ctx->whatSort == IS_SAYAT)) {
			SetActorTalking(_ctx->actor, false);
			if (IsTaggedActor(_ctx->actor))
				CORO_INVOKE_ARGS(ActorEvent, (CORO_SUBCTX, _ctx->actor, ENDTALK, false, 0));
			else if (IsTagPolygon(_ctx->actor | ACTORTAG_KEY))
				CORO_INVOKE_ARGS(PolygonEvent, (CORO_SUBCTX,
					GetTagHandle(_ctx->actor | ACTORTAG_KEY), ENDTALK, 0, false, 0));

			CORO_SLEEP(1);
		}
	} else {
		_vm->_mixer->stopHandle(_ctx->handle);
	}

	/*
	 * Restore user control and tags, as appropriate
	 * And, finally, release the talk token.
	 */
	if (_ctx->bTookControl) {
		if (TinselV2) ControlOn(); else Control(CONTROL_ON);
	}
	if (_ctx->bTookTags)
		EnableTags();

	CORO_END_CODE;
}

/**
 * TalkAt(actor, x, y, text)
 */
static void TalkAt(CORO_PARAM, int actor, int x, int y, SCNHANDLE text, bool escOn, int myEscape) {
	if (!coroParam) {
		// Don't do it if it's not wanted
		if (escOn && myEscape != GetEscEvents())
			return;

		if (!isJapanMode() && (_vm->_config->_useSubtitles || !_vm->_sound->sampleExists(text)))
			SetTextPal(GetActorRGB(actor));
	}

	Print(coroParam, x, y, text, 0, false, escOn, myEscape);
}

/**
 * TalkAtS(actor, x, y, text, sustain)
 */
static void TalkAtS(CORO_PARAM, int actor, int x, int y, SCNHANDLE text, int sustain, bool escOn, int myEscape) {
	if (!coroParam) {
		assert(sustain == 2);

		// Don't do it if it's not wanted
		if (escOn && myEscape != GetEscEvents())
			return;

		if (!isJapanMode())
			SetTextPal(GetActorRGB(actor));
	}

	Print(coroParam, x, y, text, 0, sustain == 2, escOn, myEscape);
}

/**
 * Set talk font's palette entry.
 */
static void TalkAttr(int r1, int g1, int b1, bool escOn, int myEscape) {
	if (isJapanMode())
		return;

	// Don't do it if it's not wanted
	if (escOn && myEscape != GetEscEvents())
		return;

	if (r1 > MAX_INTENSITY)	r1 = MAX_INTENSITY;	// } Ensure
	if (g1 > MAX_INTENSITY)	g1 = MAX_INTENSITY;	// } within limits
	if (b1 > MAX_INTENSITY)	b1 = MAX_INTENSITY;	// }

	SetTextPal(TINSEL_RGB(r1, g1, b1));
}

/**
 * TalkPaletteIndex
 */
static void TalkPaletteIndex(unsigned index) {
	assert(index);

	SetTalkTextOffset(index);
}

/**
 * Set talk font's palette entry.
 */
static void TalkRGB(COLORREF color, int myescEvent) {
	// Don't do it if it's not wanted
	if (myescEvent && myescEvent != GetEscEvents())
		return;

	SetTextPal(color);
}

/**
 * TalkVia("actor"/off)
 */
static void TalkVia(int actor) {
	SetSysVar(ISV_DIVERT_ACTOR, actor);
}

/**
 * Declare a temporary text font.
 */
static void TempTagFont(SCNHANDLE hFilm) {
	SetTempTagFontHandle(hFilm);	// Store the font handle
}

/**
 * Declare a temporary text font.
 */
static void TempTalkFont(SCNHANDLE hFilm) {
	SetTempTalkFontHandle(hFilm);	// Store the font handle
}

/**
 * ThisObject
 */
static int ThisObject(INV_OBJECT *pinvo) {
	assert(pinvo != NULL);

	return pinvo->id;
}

/**
 * ThisTag
 */
static int ThisTag(HPOLYGON hp) {
	int tagno;

	assert(hp != NOPOLY);

	tagno = GetTagPolyId(hp);

	assert(IsTagPolygon(tagno));
	assert(tagno);

	return tagno;
}

/**
 * Get a timer's current count.
 */
static int TimerFn(int timerno) {
	return Timer(timerno);
}

/**
 * Return the icon that caused the CONVERSE event.
 */
int Topic() {
	return GetIcon();
}

/**
 * topplay(film, x, y, actor, hold, complete)
 */
static void TopPlay(CORO_PARAM, SCNHANDLE film, int x, int y, int complete, int actorid, bool splay, int sfact, bool escOn, int myescTime) {
	Play(coroParam, film, x, y, complete, actorid, splay, sfact, escOn, myescTime, true);
}
static void TopPlay(CORO_PARAM, SCNHANDLE hFilm, int x, int y, bool bComplete, int myescEvent, TINSEL_EVENT event) {
	Play(coroParam, hFilm, x, y, bComplete, myescEvent, true, event, NOPOLY, 0);
}

/**
 * Open or close the 'top window'
 */
static void TopWindow(int bpos) {
	bool isStart = (TinselV2 && (bpos != 0)) || (!TinselV2 && (bpos == TW_START));

	KillInventory();

	if (isStart)
		OpenMenu(TOP_WINDOW);
}

/**
 * TranslucentIndex
 */
static void TranslucentIndex(unsigned index) {
	assert(index <= 255);

	SetTranslucencyOffset(index);
}

/**
 * Play a sample.
 */
static void TryPlaySample(CORO_PARAM, int sample, bool bComplete, bool escOn, int myEscape) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	// Don't do it if it's not appropriate
	if (_vm->_sound->sampleIsPlaying()) {
		// return, but prevent Glitter lock-up
		CORO_SLEEP(1);
		return;
	}

	CORO_INVOKE_ARGS(PlaySample, (CORO_SUBCTX, sample, bComplete, escOn, myEscape));
	CORO_END_CODE;
}

/**
 * UnDimMusic
 */
static void UnDimMusic() {
	_vm->_pcmMusic->unDim(true);
}

/**
 * unhookscene
 */
static void UnHookSceneFn() {
	UnHookScene();
}

/**
 * Un-define an actor as tagged.
 */
static void UnTagActorFn(int actor) {
	UnTagActor(actor);
}

/**
 * vibrate
 */
static void Vibrate() {
}

/**
 * waitframe(int actor, int frameNumber)
 */
static void WaitFrame(CORO_PARAM, int actor, int frameNumber, bool escOn, int myEscape) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	while (GetActorSteps(actor) < frameNumber) {
		// Don't do it if it's not wanted
		if (escOn && myEscape != GetEscEvents())
			break;

		CORO_SLEEP(1);
	}

	CORO_END_CODE;
}

/**
 * Return when a key pressed or button pushed.
 */
static void WaitKey(CORO_PARAM, bool escOn, int myEscape) {
	CORO_BEGIN_CONTEXT;
		int	startEvent;
		int startX, startY;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Don't do it if it's not wanted
	if (escOn && myEscape != GetEscEvents())
		return;

	for (;;) {
		_ctx->startEvent = getUserEvents();
		if (TinselV1) {
			// Store cursor position
			while (!GetCursorXYNoWait(&_ctx->startX, &_ctx->startY, false))
				CORO_SLEEP(1);
		}

		while (_ctx->startEvent == getUserEvents()) {
			CORO_SLEEP(1);

			// Not necessary to monitor escape as it's an event anyway
			if (TinselV1) {
				int curX, curY;
				GetCursorXY(&curX, &curY, false);	// Store cursor position
				if (curX != _ctx->startX || curY != _ctx->startY)
					break;
			}

			if (MenuActive())
				break;
		}

		if (!MenuActive())
			return;

		do {
			CORO_SLEEP(1);
		} while (MenuActive());

		CORO_SLEEP(ONE_SECOND / 2);		// Let it die down
	}
	CORO_END_CODE;
}

/**
 * Return when no scrolling is going on.
 */
void WaitScroll(CORO_PARAM, int myescEvent) {
	CORO_BEGIN_CONTEXT;
		int time;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// wait for ongoing scroll
	while (IsScrolling()) {
		if (myescEvent && myescEvent != GetEscEvents())
			break;

		CORO_SLEEP(1);
	}

	CORO_END_CODE;
}

/**
 * Pause for requested time.
 */
static void WaitTime(CORO_PARAM, int time, bool frame, bool escOn, int myEscape) {
	CORO_BEGIN_CONTEXT;
		int time;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Don't do it if it's not wanted
	if (escOn && myEscape != GetEscEvents())
		return;

	if (!frame)
		time *= ONE_SECOND;

	_ctx->time = time;
	do {
		CORO_SLEEP(1);

		// Abort if escapable and ESCAPE is pressed
		if (escOn && myEscape != GetEscEvents())
			break;
	} while (_ctx->time--);

	CORO_END_CODE;
}

/**
 * Set a moving actor off on a walk.
 */
void Walk(CORO_PARAM, int actor, int x, int y, SCNHANDLE hFilm, int hold, bool igPath,
		  int zOverride, bool escOn, int myescEvent) {
	CORO_BEGIN_CONTEXT;
		int thisWalk;
	CORO_END_CONTEXT(_ctx);

	bool bQuick = hold != 0;
	PMOVER pMover = GetMover(actor);

	assert(pMover); // Can't walk a non-moving actor

	CORO_BEGIN_CODE(_ctx);

	// Straight there if escaped
	if (escOn && myescEvent != GetEscEvents()) {
		if (TinselV2)
			StopMover(pMover);
		CORO_INVOKE_ARGS(Stand, (CORO_SUBCTX, actor, x, y, 0));
		return;
	}

	if (TinselV2) {
		if (MoverHidden(pMover))
			return;

		// Test 10/10/96
		while (!MoverIs(pMover))
			CORO_SLEEP(1);
	}

	assert(pMover->hCpath != NOPOLY); // moving actor not in path

	// Croak if he is doing an SWalk()
	if (TinselV2) {
		// Croak if he is doing an SWalk()
		if (MoverIsSWalking(pMover))
			CORO_KILL_SELF();

		_ctx->thisWalk = SetActorDest(pMover, x, y, igPath, hFilm);
		SetMoverZoverride(pMover, zOverride);
		DontScrollCursor();

		if (!bQuick) {
			while (MoverMoving(pMover)) {
				// Straight there if escaped
				if (escOn && myescEvent != GetEscEvents()) {
					StopMover(pMover);
					CORO_INVOKE_ARGS(Stand, (CORO_SUBCTX, actor, x, y, 0));
					break;
				}

				CORO_SLEEP(1);

				// Die if superceded
				if (_ctx->thisWalk != GetWalkNumber(pMover))
					CORO_KILL_SELF();
			}
		}
	} else {

		GetToken(pMover->actorToken);
		SetActorDest(pMover, x, y, igPath, hFilm);
		DontScrollCursor();

		if (hold == 2) {
			;
		} else {
			while (MoverMoving(pMover)) {
				CORO_SLEEP(1);

				// Straight there if escaped
				if (escOn && myescEvent != GetEscEvents()) {
					CORO_INVOKE_ARGS(Stand, (CORO_SUBCTX, actor, x, y, 0));
					FreeToken(pMover->actorToken);
					return;
				}
			}
		}

		FreeToken(pMover->actorToken);
	}

	CORO_END_CODE;
}

/**
 * Set a moving actor off on a walk.
 * Wait to see if its aborted or completed.
 */
static void Walked(CORO_PARAM, int actor, int x, int y, SCNHANDLE film, bool escOn, int myEscape, bool &retVal) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		int	thisWalk;
	CORO_END_CONTEXT(_ctx);

	PMOVER pMover = GetMover(actor);
	assert(pMover); // Can't walk a non-moving actor

	CORO_BEGIN_CODE(_ctx);

	// Straight there if escaped
	if (escOn && myEscape != GetEscEvents()) {
		CORO_INVOKE_ARGS(Stand, (CORO_SUBCTX, actor, x, y, 0));
		retVal = true;
		return;
	}

	if (TinselV2) {
		if (MoverHidden(pMover) || !MoverIs(pMover)) {
			retVal = false;
			return;
		}
		assert(pMover->hCpath != NOPOLY); // moving actor not in path

		// Not if he is doing an SWalk()
		if (MoverIsSWalking(pMover)) {
			retVal = false;
			return;
		}

	} else {
		// Pause before starting the walk
		CORO_SLEEP(ONE_SECOND);

		assert(pMover->hCpath != NOPOLY); // moving actor not in path

		// Briefly aquire token to kill off any other normal walk
		GetToken(pMover->actorToken);
		FreeToken(pMover->actorToken);
	}

	_ctx->thisWalk = SetActorDest(pMover, x, y, false, film);
	DontScrollCursor();

	while (MoverMoving(pMover) && (_ctx->thisWalk == GetWalkNumber(pMover))) {
		// Straight there if escaped
		if (escOn && myEscape != GetEscEvents()) {
			CORO_INVOKE_ARGS(Stand, (CORO_SUBCTX, actor, x, y, 0));
			retVal = true;
			return;
		}

		CORO_SLEEP(1);
	}

	int	endx, endy;
	GetMoverPosition(pMover, &endx, &endy);
	retVal = (_ctx->thisWalk == GetWalkNumber(pMover) && endx == x && endy == y);

	CORO_END_CODE;
}

/**
 * Declare a moving actor.
 */
static void WalkingActor(uint32 id, SCNHANDLE *rp = NULL) {
	PMOVER	pActor;		// Moving actor structure

	if (TinselVersion == TINSEL_V2) {
		RegisterMover(id);
		return;
	}

	RegisterMover(id);		// Establish as a moving actor
	pActor = GetMover(id);
	assert(pActor);

	// Store all those reels
	int i, j;
	for (i = 0; i < 5; ++i) {
		for (j = 0; j < 4; ++j)
			pActor->walkReels[i][j] = *rp++;
		for (j = 0; j < 4; ++j)
			pActor->standReels[i][j] = *rp++;
	}


	for (i = NUM_MAINSCALES; i < TOTAL_SCALES; i++) {
		for (j = 0; j < 4; ++j) {
			pActor->walkReels[i][j] = pActor->walkReels[4][j];
			pActor->standReels[i][j] = pActor->standReels[2][j];
		}
	}
}

/**
 * Walk a moving actor towards the polygon's tag, but return when the
 * actor enters the polygon.
 */
static void WalkPoly(CORO_PARAM, int actor, SCNHANDLE film, HPOLYGON hp, bool escOn, int myEscape) {
	int	pnodex, pnodey;

	// COROUTINE
	CORO_BEGIN_CONTEXT;
		int thisWalk;
	CORO_END_CONTEXT(_ctx);

	assert(hp != NOPOLY); // WalkPoly() may only be called from a polygon code block
	PMOVER pMover = GetMover(actor);
	assert(pMover); // Can't walk a non-moving actor

	CORO_BEGIN_CODE(_ctx);

	// Straight there if escaped
	if (escOn && myEscape != GetEscEvents()) {
		StandTag(actor, hp);
		return;
	}

	if (TinselV2) {
		if (MoverHidden(pMover))
			return;

		// Croak if he is doing an SWalk()
		if (MoverIsSWalking(pMover))
			CORO_KILL_SELF();

	} else {
		GetToken(pMover->actorToken);
	}

	GetPolyNode(hp, &pnodex, &pnodey);
	_ctx->thisWalk = SetActorDest(pMover, pnodex, pnodey, false, film);
	DoScrollCursor();

	while (!MoverIsInPolygon(pMover, hp) && MoverMoving(pMover)) {
		CORO_SLEEP(1);

		if (escOn && myEscape != GetEscEvents()) {
			// Straight there if escaped
			StandTag(actor, hp);
			if (!TinselV2)
				FreeToken(pMover->actorToken);
			return;
		}

		// Die if superceded
		if (TinselV2 && (_ctx->thisWalk != GetWalkNumber(pMover)))
			CORO_KILL_SELF();
	}

	if (!TinselV2)
		FreeToken(pMover->actorToken);

	CORO_END_CODE;
}

/**
 * WalkTag(actor, reel, hold)
 */
static void WalkTag(CORO_PARAM, int actor, SCNHANDLE film, HPOLYGON hp, bool escOn, int myEscape) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		int thisWalk;
	CORO_END_CONTEXT(_ctx);

	PMOVER pMover = GetMover(actor);
	assert(pMover); // Can't walk a non-moving actor

	CORO_BEGIN_CODE(_ctx);

	int	pnodex, pnodey;

	assert(hp != NOPOLY); // walkpoly() may only be called from a polygon code block

	// Straight there if escaped
	if (escOn && myEscape != GetEscEvents()) {
		StandTag(actor, hp);
		return;
	}

	if (!TinselV2)
		GetToken(pMover->actorToken);
	else {
		if (MoverHidden(pMover))
			return;
	}

	GetPolyNode(hp, &pnodex, &pnodey);

	_ctx->thisWalk = SetActorDest(pMover, pnodex, pnodey, false, film);
	DoScrollCursor();

	while (MoverMoving(pMover)) {
		if (escOn && myEscape != GetEscEvents()) {
			// Straight there if escaped
			StandTag(actor, hp);
			if (!TinselV2)
				FreeToken(pMover->actorToken);
			return;
		}

		CORO_SLEEP(1);

		// Die if superceded
		if (TinselV2 && (_ctx->thisWalk != GetWalkNumber(pMover)))
			CORO_KILL_SELF();
	}

	// Adopt the tag-related reel
	SCNHANDLE pFilm = GetPolyFilm(hp);

	switch (pFilm) {
	case TF_NONE:
		break;

	case TF_UP:
		SetMoverDirection(pMover, AWAY);
		SetMoverStanding(pMover);
		break;
	case TF_DOWN:
		SetMoverDirection(pMover, FORWARD);
		SetMoverStanding(pMover);
		break;
	case TF_LEFT:
		SetMoverDirection(pMover, LEFTREEL);
		SetMoverStanding(pMover);
		break;
	case TF_RIGHT:
		SetMoverDirection(pMover, RIGHTREEL);
		SetMoverStanding(pMover);
		break;

	default:
		if (actor == LEAD_ACTOR || actor == GetLeadId())
			AlterMover(pMover, pFilm, AR_NORMAL);
		else
			SetMoverStanding(pMover);
		break;
	}

	if (!TinselV2)
		FreeToken(pMover->actorToken);

	CORO_END_CODE;
}

/**
 * Returns the X co-ordinateof lead actor's last walk.
 */
int WalkXPos() {
	return GetLastLeadXdest();
}

/**
 * Returns the Y co-ordinateof lead actor's last walk.
 */
int WalkYPos() {
	return GetLastLeadYdest();
}

/**
 * Return which is the current CD, counting from 1.
 */
int WhichCd() {
	return GetCurrentCD();
}

/**
 * whichinventory
 */
int WhichInventory() {
	return WhichInventoryOpen();
}


/**
 * Subtract one less that the number of parameters from pp
 * pp then points to the first parameter.
 *
 * If the library function has no return value:
 * return -(the number of parameters) to pop them from the stack
 *
 * If the library function has a return value:
 * return -(the number of parameters - 1) to pop most of them from
 * the stack, and stick the return value in pp[0]
 * @param operand			Library function
 * @param pp				Top of parameter stack
 */
int CallLibraryRoutine(CORO_PARAM, int operand, int32 *pp, const INT_CONTEXT *pic, RESUME_STATE *pResumeState) {
	int libCode;
	if (TinselV0) libCode = DW1DEMO_CODES[operand];
	else if (!TinselV2) libCode = DW1_CODES[operand];
	else if (TinselV2Demo) libCode = DW2DEMO_CODES[operand];
	else libCode = DW2_CODES[operand];

	debug(7, "CallLibraryRoutine op %d (escOn %d, myEscape %d)", operand, pic->escOn, pic->myEscape);
	switch (libCode) {
	case ACTORATTR:
		// DW1 only
		pp -= 3;			// 4 parameters
		ActorAttr(pp[0], pp[1], pp[2], pp[3]);
		return -4;

	case ACTORBRIGHTNESS:
		// DW2 only
		pp -= 1;
		ActorBrightness(pp[0], pp[1]);
		return -2;

	case ACTORDIRECTION:
		// Common to both DW1 & DW2
		pp[0] = ActorDirection(pp[0]);
		return 0;

	case ACTORPALETTE:
		// DW2 only
		pp -= 2;			// 3 parameters
		ActorPalette(pp[0], pp[1], pp[2]);
		return -3;

	case ACTORPRIORITY:
		// DW2 only
		pp -= 1;			// 2 parameters
		ActorPriority(pp[0], pp[1]);
		return -2;

	case ACTORREF:
		// Common to both DW1 & DW2
		if (!TinselV0)
			error("actorref isn't a real function");
		return 0;

	case ACTORRGB:
		// DW2 only
		pp -= 1;			// 2 parameters
		ActorRGB(pp[0], pp[1]);
		return -2;

	case ACTORSCALE:
		// Common to both DW1 & DW2
		pp[0] = ActorScale(pp[0]);
		return 0;

	case ACTORSON:
		// DW1 only
		ActorsOn();
		return 0;

	case ACTORXPOS:
		// Common to both DW1 & DW2
		pp[0] = ActorPos(ACTORXPOS, pp[0]);
		return 0;

	case ACTORYPOS:
		// Common to both DW1 & DW2
		pp[0] = ActorPos(ACTORYPOS, pp[0]);
		return 0;

	case ADDHIGHLIGHT:
		// DW2 only
		// Command doesn't actually do anything
		pp -= 1;			// 2 parameters
		return -2;

	case ADDINV:
		// DW2 only
		AddInv(INV_DEFAULT, pp[0]);
		return -1;

	case ADDINV1:
		// Common to both DW1 & DW2
		AddInv(INV_1, pp[0]);
		return -1;

	case ADDINV2:
		// Common to both DW1 & DW2
		AddInv(INV_2, pp[0]);
		return -1;

	case ADDOPENINV:
		// Common to both DW1 & DW2
		AddInv(TinselV2 ? DW2_INV_OPEN : INV_OPEN, pp[0]);
		return -1;

	case ADDTOPIC:
		// Common to both DW1 & DW2
		AddTopic(pp[0]);
		return -1;

	case AUXSCALE:
		// DW1 only
		pp -= 13;			// 14 parameters
		AuxScale(pp[0], pp[1], (SCNHANDLE *)(pp+2));
		return -14;

	case BACKGROUND:
		// Common to both DW1 & DW2
		Background(coroParam, pp[0]);
		return -1;

	case BLOCKING:
		// DW2 only
		Blocking(pp[0]);
		return -1;

	case CALLACTOR:
	case CALLGLOBALPROCESS:
	case CALLOBJECT:
		// DW2 only
		pp -= 1;			// 2 parameters
		if (*pResumeState == RES_1 && pic->resumeCode == RES_WAITING) {
			bool result;
			*pResumeState = RES_NOT;
			FinishWaiting(coroParam, pic, &result);
			if (coroParam) {
				*pResumeState = RES_1;
				return 0;
			}
			pp[0] = result ? 1 : 0;
		} else {
			uint32 v;
			if (libCode == CALLACTOR)
				v = SendActor(coroParam, pp[0], (TINSEL_EVENT)pp[1], pic->hPoly, pic->myEscape);
			else if (libCode == CALLGLOBALPROCESS)
				v = SendGlobalProcess(coroParam, pp[0], (TINSEL_EVENT)pp[1], pic->myEscape);
			else
				v = SendObject(coroParam, pp[0], (TINSEL_EVENT)pp[1], pic->myEscape);

			if (coroParam)
				return 0;
			pp[0] = v;
		}

		if (!pp[0])
			KillSelf(coroParam);
		return -2;


	case CALLPROCESS:
		// DW2 only
		pp -= 1;			// 2 parameters
		if (*pResumeState == RES_1 && pic->resumeCode == RES_WAITING) {
			bool result;
			*pResumeState = RES_NOT;
			FinishWaiting(coroParam, pic, &result);
			if (coroParam) {
				*pResumeState = RES_1;
				return 0;
			}

			pp[0] = result ? 1 : 0;
		} else {
			int result = SendProcess(coroParam, pp[0], (TINSEL_EVENT)pp[1], pic->myEscape);
			if (coroParam)
				return 0;

			pp[0] = result;
		}
		return -2;

	case CALLSCENE:
		// DW2 only
		error("CallScene isn't a real function");

	case CALLTAG:
		// DW2 only
		pp -= 1;			// 2 parameters
		if (*pResumeState == RES_1 && pic->resumeCode == RES_WAITING) {
			bool result;
			*pResumeState = RES_NOT;
			FinishWaiting(coroParam, pic, &result);
			if (coroParam) {
				*pResumeState = RES_1;
				return 0;
			}

			pp[0] = result ? 1 : 0;
		} else {
			bool result;
			SendTag(coroParam, pp[0], (TINSEL_EVENT)pp[1], pic->hPoly, pic->myEscape, &result);
			if (coroParam)
				return 0;

			pp[0] = result ? 1 : 0;
		}

		if (!pp[0])
			KillSelf(coroParam);
		return -2;

	case CAMERA:
		// Common to both DW1 & DW2
		Camera(pp[0]);
		return -1;

	case CDCHANGESCENE:
		// DW2 only
		CdChangeScene(pp[0]);
		return -1;

	case CDDOCHANGE:
		// DW2 only
		CdDoChange(coroParam);
		return 0;

	case CDENDACTOR:
		// DW2 only
		CdEndActor(pp[0], pic->myEscape);
		return -1;

	case CDLOAD:
		// Common to both DW1 & DW2
		pp -= 1;			// 2 parameters
		CDload(pp[0], pp[1], pic->myEscape);
		return -2;

	case CDPLAY:
		// Common to both DW1 & DW2
		error("cdplay isn't a real function");

	case CLEARHOOKSCENE:
		// Common to both DW1 & DW2
		ClearHookScene();
		return 0;

	case CLOSEINVENTORY:
		// Common to both DW1 & DW2
		CloseInventory();
		return 0;

	case CONTROL:
		// Common to both DW1 & DW2
		Control(pp[0]);
		return -1;

	case CONVERSATION:
		// Common to both DW1 & DW2
		Conversation(coroParam, pp[0], pic->hPoly, pic->idActor, pic->escOn, pic->myEscape);
		return -1;

	case CONVTOPIC:
		// Common to both DW1 & DW2
		ConvTopic(pp[0]);
		return -1;

	case CURSOR:
		// DW2 only
		Cursor(pp[0]);
		return -1;

	case CURSORXPOS:
		// Common to both DW1 & DW2
		pp[0] = CursorPos(CURSORXPOS);
		return 0;

	case CURSORYPOS:
		// Common to both DW1 & DW2
		pp[0] = CursorPos(CURSORYPOS);
		return 0;

	case CUTSCENE:
		// DW1 only
		error("cutscene isn't a real function");

	case DECCONVW:
		// Common to both DW1 & DW2
		pp -= 7;			// 8 parameters
		DecConvW(pp[0], pp[1], pp[2], pp[3],
			 pp[4], pp[5], pp[6], pp[7]);
		return -8;

	case DECCSTRINGS:
		// DW1 only
		pp -= 19;			// 20 parameters
		DecCStrings((SCNHANDLE *)pp);
		return -20;

	case DECCURSOR:
		// Common to both DW1 & DW2
		DecCursor(pp[0]);
		return -1;

	case DECFLAGS:
		// Common to both DW1 & DW2
		if (TinselV2)
			error("DecFlags() is obsolete");

		DecFlags(pp[0]);
		return -1;

	case DECINV1:
		// Common to both DW1 & DW2
		pp -= 7;			// 8 parameters
		DecInv1(pp[0], pp[1], pp[2], pp[3],
			 pp[4], pp[5], pp[6], pp[7]);
		return -8;

	case DECINV2:
		// Common to both DW1 & DW2
		pp -= 7;			// 8 parameters
		DecInv2(pp[0], pp[1], pp[2], pp[3],
			 pp[4], pp[5], pp[6], pp[7]);
		return -8;

	case DECINVW:
		// Common to both DW1 & DW2
		DecInvW(pp[0]);
		return -1;

	case DECLARELANGUAGE:
		// DW2 only
		pp -= 2;			// 3 parameters
		DeclareLanguage(pp[0], pp[1], pp[2]);
		return -3;

	case DECLEAD:
		// Common to both DW1 & DW2
		if (TinselV2) {
			DecLead(pp[0]);
			return -1;
		} else {
			pp -= 61;			// 62 parameters
			DecLead(pp[0], (SCNHANDLE *)&pp[1], pp[61]);
			return -62;
		}

	case DECSCALE:
		// DW2 only
		pp -= 13;			// 14 parameters
		DecScale(pp[0], pp[1], pp[2], pp[3], pp[4],
			 pp[5], pp[6], pp[7], pp[8], pp[9],
			 pp[10], pp[11], pp[12], pp[13]);
		return -14;

	case DECTAGFONT:
		// Common to both DW1 & DW2
		DecTagFont(pp[0]);
		return -1;

	case DECTALKFONT:
		// Common to both DW1 & DW2
		DecTalkFont(pp[0]);
		return -1;

	case DELICON:
		// DW1 only
		DelIcon(pp[0]);
		return -1;

	case DELINV:
		// DW1 only
		DelInv(pp[0]);
		return -1;

	case DELTOPIC:
		// DW2 only
		DelTopic(pp[0]);
		return -1;

	case DIMMUSIC:
		// DW2 only
		DimMusic();
		return 0;

	case DROP:
		// DW2 only
		Drop(pp[0]);
		return -1;

	case DROPEVERYTHING:
		// DW2 only
		DropEverything();
		return 0;

	case DROPOUT:
		// DW1 only
		error("DropOut (%d)", pp[0]);

	case EFFECTACTOR:
		// Common to both DW1 & DW2
		assert(pic->event == WALKIN || pic->event == WALKOUT); // effectactor() must be from effect poly code

		pp[0] = pic->idActor;
		return 0;

	case ENABLEMENU:
		// Common to both DW1 & DW2
		EnableMenu();
		return 0;

	case ENDACTOR:
		// DW2 only
		EndActor(pp[0]);
		return -1;

	case ESCAPE:
	case ESCAPEOFF:
	case ESCAPEON:
		// Common to both DW1 & DW2
		error("Escape isn't a real function");

	case EVENT:
		// Common to both DW1 & DW2
		if (TinselVersion == TINSEL_V2)
			pp[0] = pic->event;
		else
			pp[0] = TINSEL1_EVENT_MAP[pic->event];
		return 0;

	case FACETAG:
		// DW2 only
		FaceTag(pp[0], pic->hPoly);
		return -1;

	case FADEIN:
		// DW2 only
		FadeIn();
		return 0;

	case FADEMIDI:
		// DW1 only
		FadeMidi(coroParam, pp[0]);
		return -1;

	case FADEOUT:
		// DW1 only
		FadeOut();
		return 0;

	case FRAMEGRAB:
		// Common to both DW1 & DW2
		return -1;

	case FREEZECURSOR:
		// DW2 only
		FreezeCursor(pp[0]);
		return -1;

	case GETINVLIMIT:
		// Common to both DW1 & DW2
		pp[0] = GetInvLimit(pp[0]);
		return 0;

	case GHOST:
		// DW2 only
		pp -= 2;			// 3 parameters
		Ghost(pp[0], pp[1], pp[2]);
		return -3;

	case GLOBALVAR:
		// DW1 only
		error("GlobalVar isn't a real function");

	case GRABMOVIE:
		// DW2 only
		return -1;

	case HAILSCENE:
		// DW2 only
		HailScene(pp[0]);
		return -1;

	case HASRESTARTED:
		// Common to both DW1 & DW2
		pp[0] = HasRestarted();
		return 0;

	case HAVE:
		// DW2 only
		pp[0] = Have(pp[0]);
		return 0;			// using return value

	case HELDOBJECT:
		// Common to both DW1 & DW2
		pp[0] = HeldObject();
		return 0;

	case HIDEACTOR:
		// Common to both DW1 & DW2
		if (!TinselV2)
			HideActorFn(coroParam, pp[0]);
		else if (*pResumeState == RES_1 && pic->resumeCode == RES_WAITING) {
			*pResumeState = RES_NOT;
			FinishWaiting(coroParam, pic);
			if (coroParam) {
				*pResumeState = RES_1;
				return 0;
			}
		} else
			HideActorFn(coroParam, pp[0]);
		return -1;

	case HIDEBLOCK:
		// DW2 only
		HideBlock(pp[0]);
		return -1;

	case HIDEEFFECT:
		// DW2 only
		HideEffect(pp[0]);
		return -1;

	case HIDEPATH:
		// DW2 only
		HidePath(pp[0]);
		return -1;

	case HIDEREFER:
		// DW2 only
		HideRefer(pp[0]);
		return -1;

	case HIDETAG:
		// DW2 only
		if (*pResumeState == RES_1 && pic->resumeCode == RES_WAITING) {
			*pResumeState = RES_NOT;
			FinishWaiting(coroParam, pic);
			if (coroParam) {
				*pResumeState = RES_1;
				return 0;
			}
		} else {
			HideTag(coroParam, pp[0], pic->hPoly);
			if (coroParam)
				return 0;
		}
		return -1;

	case HOLD:
		// DW2 only
		Hold(pp[0]);
		return -1;

	case HOOKSCENE:
		// Common to both DW1 & DW2
		pp -= 2;			// 3 parameters
		HookScene(pp[0], pp[1], pp[2]);
		return -3;

	case IDLETIME:
		// Common to both DW1 & DW2
		pp[0] = IdleTime();
		return 0;

	case ININVENTORY:
		// DW1 only
		pp[0] = InInventory(pp[0]);
		return 0;			// using return value

	case INSTANTSCROLL:
		// DW2 only
		InstantScroll(pp[0]);
		return -1;

	case INVDEPICT:
		// DW1 only
		pp -= 1;			// 2 parameters
		InvDepict(pp[0], pp[1]);
		return -2;

	case INVENTORY:
		// Common to both DW1 & DW2
		Inventory(pp[0], pic->escOn, pic->myEscape);
		return -1;

	case INVPLAY:
		// DW2 only
		pp -= 1;			// 2 parameters
		InvPlay(pp[0], pp[1]);
		return -2;

	case INWHICHINV:
		// Common to both DW1 & DW2
		pp[0] = InWhichInv(pp[0]);
		return 0;			// using return value

	case KILLACTOR:
		// DW1 only
		if (TinselV2)
			error("KillActor() was not expected to be required");

		KillActor(pp[0]);
		return -1;

	case KILLBLOCK:
		// DW1 only
		KillBlock(pp[0]);
		return -1;

	case KILLEXIT:
		// DW1 only
		KillExit(pp[0]);
		return -1;

	case KILLGLOBALPROCESS:
		// DW2 only
		KillGlobalProcess(pp[0]);
		return -1;

	case KILLPROCESS:
		// DW2 only
		KillProcess(pp[0]);
		return -1;

	case KILLTAG:
		// DW1 only
		KillTag(coroParam, pp[0]);
		return -1;

	case LOCALVAR:
		// DW2 only
		error("LocalVar isn't a real function");

	case MOVECURSOR:
		// Common to both DW1 & DW2
		pp -= 1;			// 2 parameters
		MoveCursor(pp[0], pp[1]);
		return -2;

	case MOVETAG:
		// DW2 only
		pp -= 2;			// 3 parameters
		MoveTag(pp[0], pp[1], pp[2], pic->hPoly);
		return -3;

	case MOVETAGTO:
		// DW2 only
		pp -= 2;			// 3 parameters
		MoveTagTo(pp[0], pp[1], pp[2], pic->hPoly);
		return -3;

	case NEWSCENE:
		// Common to both DW1 & DW2
		pp -= 2;			// 3 parameters
		if (*pResumeState == RES_2)
			*pResumeState = RES_NOT;
		else
			NewScene(coroParam, pp[0], pp[1], pp[2]);
		return -3;

	case NOBLOCKING:
		// Common to both DW1 & DW2
		NoBlocking();
		return 0;

	case NOPAUSE:
		// DW2 only
		g_bNoPause = true;
		return 0;

	case NOSCROLL:
		// Common to both DW1 & DW2
		pp -= 3;			// 4 parameters
		NoScroll(pp[0], pp[1], pp[2], pp[3]);
		return -4;

	case OBJECTHELD:
		// DW1 only
		ObjectHeld(pp[0]);
		return -1;

	case OFFSET:
		// Common to both DW1 & DW2
		if (TinselV2) {
			pp -= 2;			// 2 parameters
			Offset((EXTREME)pp[0], pp[1], pp[2]);
			return -3;
		} else {
			pp -= 1;			// 2 parameters
			Offset(EX_USEXY, pp[0], pp[1]);
			return -2;
		}

	case OTHEROBJECT:
		// DW2 only
		pp[0] = OtherObject(pic->pinvo);
		return 0;

	case PAUSE:
		// DW2 only
		WaitTime(coroParam, 1, true, pic->escOn, pic->myEscape);
		return 0;

	case PLAY:
		// Common to both DW1 & DW2
		if (TinselV2) {
			pp -= 3;			// 4 parameters
			if (*pResumeState == RES_1 && IsCdPlayHandle(pp[0]))
				*pResumeState = RES_NOT;
			else {
				Play(coroParam, pp[0], pp[1], pp[2], pp[3], pic->myEscape, false,
						pic->event, pic->hPoly, pic->idActor);
			}
			return -4;

		} else {
			pp -= 5;			// 6 parameters

			if (pic->event == WALKIN || pic->event == WALKOUT)
				Play(coroParam, pp[0], pp[1], pp[2], pp[5], 0, false, 0, pic->escOn, pic->myEscape, false);
			else
				Play(coroParam, pp[0], pp[1], pp[2], pp[5], pic->idActor, false, 0, pic->escOn, pic->myEscape, false);
			return -6;
		}

	case PLAYMIDI:
		// Common to both DW1 & DW2
		pp -= 2;			// 3 parameters
		PlayMidi(coroParam, pp[0], pp[1], pp[2]);
		return -3;

	case PLAYMOVIE:
		// DW2 only
		PlayMovie(coroParam, pp[0], pic->myEscape);
		return -1;

	case PLAYMUSIC:
		// DW2 only
		PlayMusic(pp[0]);
		return -1;

	case PLAYRTF:
		// Common to both DW1 & DW2
		error("playrtf only applies to cdi");

	case PLAYSAMPLE:
		// Common to both DW1 & DW2
		if (TinselV2) {
			pp -= 3;			// 4 parameters
			PlaySample(coroParam, pp[0], pp[1], pp[2], pp[3], pic->myEscape);
			return -4;
		} else {
			pp -= 1;			// 2 parameters
			PlaySample(coroParam, pp[0], pp[1], pic->escOn, pic->myEscape);
			return -2;
		}

	case POINTACTOR:
		// DW2 only
		PointActor(pp[0]);
		return -1;

	case POINTTAG:
		// DW2 only
		PointTag(pp[0], pic->hPoly);
		return -1;

	case POSTACTOR:
		// DW2 only
		pp -= 1;			// 2 parameters
		PostActor(coroParam, pp[0], (TINSEL_EVENT)pp[1], pic->hPoly, pic->idActor, pic->myEscape);
		return -2;

	case POSTGLOBALPROCESS:
		// DW2 only
		pp -= 1;			// 2 parameters
		PostGlobalProcess(coroParam, pp[0], (TINSEL_EVENT)pp[1], pic->myEscape);
		return -2;

	case POSTOBJECT:
		// DW2 only
		pp -= 1;			// 2 parameters
		PostObject(coroParam, pp[0], (TINSEL_EVENT)pp[1], pic->myEscape);
		return -2;

	case POSTPROCESS:
		// DW2 only
		pp -= 1;			// 2 parameters
		PostProcess(coroParam, pp[0], (TINSEL_EVENT)pp[1], pic->myEscape);
		return -2;

	case POSTTAG:
		// DW2 only
		pp -= 1;			// 2 parameters
		PostTag(coroParam, pp[0], (TINSEL_EVENT)pp[1], pic->hPoly, pic->myEscape);
		return -2;


	case PREPARESCENE:
		// DW1 only
		PrepareScene(pp[0]);
		return -1;

	case PRINT:
		// Common to both DW1 & DW2
		if (TinselV2) {
			pp -= 4;			// 5 parameters
			Print(coroParam, pp[0], pp[1], pp[2], pp[3], pp[4] != 0, pic->escOn, pic->myEscape);
			return -5;
		} else {
			pp -= 5;			// 6 parameters
			/* pp[2] was intended to be attribute */
			Print(coroParam, pp[0], pp[1], pp[3], pp[4], pp[5] == 2, pic->escOn, pic->myEscape);
			return -6;
		}

	case PRINTCURSOR:
		// DW2 only
		PrintTag(pic->hPoly, pp[0], pic->idActor, true);
		return -1;

	case PRINTOBJ:
		// Common to both DW1 & DW2
		PrintObj(coroParam, pp[0], pic->pinvo, pic->event, pic->myEscape);
		return -1;

	case PRINTTAG:
		// Common to both DW1 & DW2
		PrintTag(pic->hPoly, pp[0], TinselV2 ? pic->idActor : 0, false);
		return -1;

	case QUITGAME:
		// Common to both DW1 & DW2
		QuitGame();
		return 0;

	case RANDOM:
		// Common to both DW1 & DW2
		pp -= 2;			// 3 parameters
		pp[0] = RandomFn(pp[0], pp[1], pp[2]);
		return -2;		// One holds return value

	case RESETIDLETIME:
		// Common to both DW1 & DW2
		ResetIdleTime();
		return 0;

	case RESTARTGAME:
		// Common to both DW1 & DW2
		FnRestartGame();
		return 0;

	case RESTORESCENE:
		// Common to both DW1 & DW2
		if (TinselV2) {
			RestoreScene(coroParam, (TRANSITS)pp[0]);
			return -1;
		} else {
			RestoreScene(coroParam, TRANS_FADE);
			return 0;
		}

	case RESTORE_CUT:
		// DW1 only
		RestoreScene(coroParam, TRANS_CUT);
		return 0;

	case RESUMELASTGAME:
		// DW2 only
		ResumeLastGame();
		return 0;

	case RUNMODE:
		// Common to both DW1 & DW2
		pp[0] = RunMode();
		return 0;

	case SAMPLEPLAYING:
		// DW1 only
		pp[0] = SamplePlaying(pic->escOn, pic->myEscape);
		return 0;

	case SAVESCENE:
		// Common to both DW1 & DW2
		if (*pResumeState == RES_1)
			*pResumeState = RES_2;
		else
			SaveScene(coroParam);
		return 0;

	case SAY:
		// DW2 only
		pp -= 1;			// 2 parameters
		TalkOrSay(coroParam, IS_SAY, pp[1], 0, 0, 0, pp[0], false, pic->escOn, pic->myEscape);
		return -2;

	case SAYAT:
		// DW2 only
		pp -= 4;			// 5 parameters
		TalkOrSay(coroParam, IS_SAYAT, pp[3], pp[1], pp[2], 0, pp[0], pp[4], pic->escOn, pic->myEscape);
		return -5;

	case SCALINGREELS:
		// Common to both DW1 & DW2
		pp -= 6;			// 7 parameters
		ScalingReels(pp[0], pp[1], pp[2], pp[3], pp[4], pp[5], pp[6]);
		return -7;

	case SCANICON:
		// DW1 only
		pp[0] = ScanIcon();
		return 0;

	case SCREENXPOS:
		// Common to both DW1 & DW2
		pp[0] = LToffset(SCREENXPOS);
		return 0;

	case SCREENYPOS:
		// Common to both DW1 & DW2
		pp[0] = LToffset(SCREENYPOS);
		return 0;

	case SCROLL:
		// Common to both DW1 & DW2
		if (TinselV2) {
			pp -= 5;			// 6 parameters
			Scroll(coroParam, (EXTREME)pp[0], pp[1], pp[2], pp[3], pp[4], pp[5], pic->escOn, pic->myEscape);
			return -6;
		} else {
			pp -= 3;			// 4 parameters
			Scroll(coroParam, EX_USEXY, pp[0], pp[1], pp[2], pp[2], pp[3], pic->escOn, pic->myEscape);
			return -4;
		}

	case SCROLLPARAMETERS:
		// DW2 only
		pp -= 6;			// 7 parameters
		ScrollParameters(pp[0], pp[1], pp[2], pp[3], pp[4], pp[5], pp[6]);
		return -7;

	case SENDTAG:
		// DW2 only
		pp -= 1;			// 2 parameters
		if (*pResumeState == RES_1 && pic->resumeCode == RES_WAITING) {
			bool result;
			*pResumeState = RES_NOT;
			FinishWaiting(coroParam, pic, &result);
			if (coroParam) {
				*pResumeState = RES_1;
				return 0;
			}
			pp[0] = result ? 1 : 0;
		} else {
			bool result;
			SendTag(coroParam, pp[0], (TINSEL_EVENT)pp[1], pic->hPoly, pic->myEscape, &result);
			if (coroParam)
				return 0;

			pp[0] = result;
		}
		return -1;


	case SETACTOR:
		// DW1 only
		SetActor(pp[0]);
		return -1;

	case SETBLOCK:
		// DW1 only
		SetBlock(pp[0]);
		return -1;

	case SETEXIT:
		// DW1 only
		SetExit(pp[0]);
		return -1;

	case SETINVLIMIT:
		// Common to both DW1 & DW2
		pp -= 1;			// 2 parameters
		SetInvLimit(pp[0], pp[1]);
		return -2;

	case SETINVSIZE:
		// Common to both DW1 & DW2
		pp -= 6;			// 7 parameters
		SetInvSize(pp[0], pp[1], pp[2], pp[3], pp[4], pp[5], pp[6]);
		return -7;

	case SETLANGUAGE:
		// Common to both DW1 & DW2
		SetLanguage((LANGUAGE)pp[0]);
		return -1;

	case SETPALETTE:
		// Common to both DW1 & DW2
		if (TinselV2) {
			// Note: Although DW2 introduces parameters for start and length, it doesn't use them
			pp -= 2;
			SetPalette(pp[0], pic->escOn, pic->myEscape);
			return -3;
		} else {
			SetPalette(pp[0], pic->escOn, pic->myEscape);
			return -1;
		}

	case SETSYSTEMSTRING:
		// DW2 only
		pp -= 1;				// 2 parameters
		SetSystemString(pp[0], pp[1]);
		return -2;

	case SETSYSTEMVAR:
		// DW1 only
		pp -= 1;				// 2 parameters
		SetSystemVar(pp[0], pp[1]);
		return -2;

	case SETTAG:
		// DW1 only
		SetTag(coroParam, pp[0]);
		return -1;

	case SETTIMER:
		// DW1 only
		pp -= 3;			// 4 parameters
		SetTimer(pp[0], pp[1], pp[2], pp[3]);
		return -4;

	case SHELL:
		// DW2 only
		Shell(pp[0]);
		return 0;

	case SHOWACTOR:
		// DW2 only
		if (*pResumeState == RES_1 && pic->resumeCode == RES_WAITING) {
			*pResumeState = RES_NOT;
			FinishWaiting(coroParam, pic);
			if (coroParam) {
				*pResumeState = RES_1;
				return 0;
			}
		} else
			ShowActorFn(coroParam, pp[0]);
		return -1;

	case SHOWBLOCK:
		// DW2 only
		ShowBlock(pp[0]);
		return -1;

	case SHOWEFFECT:
		// DW2 only
		ShowEffect(pp[0]);
		return -1;

	case SHOWMENU:
		// DW2 only
		ShowMenu();
		return 0;

	case SHOWPATH:
		// DW2 only
		ShowPath(pp[0]);
		return -1;

	case SHOWPOS:
		// DW1 only
#ifdef DEBUG
		showpos();
#endif
		return 0;

	case SHOWREFER:
		// DW2 only
		ShowRefer(pp[0]);
		return -1;

	case SHOWSTRING:
#ifdef DEBUG
		showstring();
#endif
		return 0;

	case SHOWTAG:
		// DW2 only
		if (*pResumeState == RES_1 && pic->resumeCode == RES_WAITING) {
			*pResumeState = RES_NOT;
			FinishWaiting(coroParam, pic);

			if (coroParam) {
				*pResumeState = RES_1;
				return 0;
			}
		} else {
			ShowTag(coroParam, pp[0], pic->hPoly);
		}
		return -1;

	case SPLAY:
		// DW1 only
		pp -= 6;			// 7 parameters

		if (pic->event == WALKIN || pic->event == WALKOUT)
			SPlay(coroParam, pp[0], pp[1], pp[2], pp[3], pp[6], 0, pic->escOn, pic->myEscape);
		else
			SPlay(coroParam, pp[0], pp[1], pp[2], pp[3], pp[6], pic->idActor, pic->escOn, pic->myEscape);
		return -7;

	case STAND:
		// Common to both DW1 & DW2
		pp -= 3;			// 4 parameters
		Stand(coroParam, pp[0], pp[1], pp[2], pp[3]);
		return -4;

	case STANDTAG:
		// Common to both DW1 & DW2
		StandTag(pp[0], pic->hPoly);
		return -1;

	case STARTGLOBALPROCESS:
		// DW2 only
		StartGlobalProcess(coroParam, pp[0]);
		return -1;

	case STARTPROCESS:
		// DW2 only
		StartProcess(coroParam, pp[0]);
		return -1;

	case STARTTIMER:
		// DW2 only
		pp -= 3;			// 4 parameters
		StartTimerFn(pp[0], pp[1], pp[2], pp[3]);
		return -4;

	case STOPMIDI:
		// DW1 only
		StopMidiFn();
		return 0;

	case STOPSAMPLE:
		// Common to both DW1 & DW2
		if (TinselV2) {
			StopSample(pp[0]);
			return -1;
		} else {
			StopSample();
			return 0;
		}

	case STOPWALK:
		// Common to both DW1 & DW2 only
		StopWalk(pp[0]);
		return -1;

	case SUBTITLES:
		// Common to both DW1 & DW2
		Subtitles(pp[0]);
		return -1;

	case SWALK:
		// Common to both DW1 & DW2
		pp -= 5;			// 6 parameters
		Swalk(coroParam, pp[0], pp[1], pp[2], pp[3], pp[4], pp[5], -1, pic->escOn, pic->myEscape);
		return -6;

	case SWALKZ:
		// DW2 only
		pp -= 6;			// 7 parameters
		Swalk(coroParam, pp[0], pp[1], pp[2], pp[3], pp[4], pp[5], pp[6], pic->escOn, pic->myEscape);
		return -7;

	case SYSTEMVAR:
		// DW2 only
		pp[0] = SystemVar(pp[0]);
		return 0;

	case TAGACTOR:
		pp -= 2;			// 3 parameters
		TagActor(pp[0], pp[1], pp[2]);
		return -3;

	case TAGTAGXPOS:
	case TAGTAGYPOS:
	case TAGWALKXPOS:
	case TAGWALKYPOS:
		// DW2 only
		pp[0] = TagPos((MASTER_LIB_CODES)libCode, pp[0], pic->hPoly);
		return 0;

	case TALK:
		// Common to both DW1 & DW2
		pp -= 1;			// 2 parameters

		if (TinselV2)
			TalkOrSay(coroParam, IS_TALK, pp[1], 0, 0, pp[0], 0, false, pic->escOn, pic->myEscape);
		else if (pic->event == WALKIN || pic->event == WALKOUT)
			TalkOrSay(coroParam, IS_TALK, pp[1], 0, 0, pp[0], 0, false, pic->escOn, pic->myEscape);
		else
			TalkOrSay(coroParam, IS_TALK, pp[1], 0, 0, pp[0], pic->idActor, false, pic->escOn, pic->myEscape);
		return -2;

	case TALKAT:
		// Common to both DW1 & DW2
		if (TinselV2) {
			pp -= 4;			// 5 parameters
			TalkOrSay(coroParam, IS_TALKAT, pp[3], pp[1], pp[2], 0, pp[0], pp[4], pic->escOn, pic->myEscape);
			return -5;
		} else {
			pp -= 3;			// 4 parameters
			TalkAt(coroParam, pp[0], pp[1], pp[2], pp[3], pic->escOn, pic->myEscape);
			return -4;
		}

	case TALKATS:
		// DW1 only
		pp -= 4;			// 5 parameters
		TalkAtS(coroParam, pp[0], pp[1], pp[2], pp[3], pp[4], pic->escOn, pic->myEscape);
		return -5;

	case TALKATTR:
		// DW1 only
		pp -= 2;			// 3 parameters
		TalkAttr(pp[0], pp[1], pp[2], pic->escOn, pic->myEscape);
		return -3;

	case TALKPALETTEINDEX:
		// DW1 only
		TalkPaletteIndex(pp[0]);
		return -1;

	case TALKRGB:
		// DW2 only
		TalkRGB(pp[0], pic->myEscape);
		return -3;

	case TALKVIA:
		// DW2 only
		TalkVia(pp[0]);
		return -1;

	case TEMPTAGFONT:
		// DW2 only
		TempTagFont(pp[0]);
		return -1;

	case TEMPTALKFONT:
		// DW2 only
		TempTalkFont(pp[0]);
		return -1;

	case THISOBJECT:
		// DW2 only
		pp[0] = ThisObject(pic->pinvo);
		return 0;

	case THISTAG:
		// DW2 only
		pp[0] = ThisTag(pic->hPoly);
		return 0;

	case TIMER:
		// Common to both DW1 & DW2
		pp[0] = TimerFn(pp[0]);
		return 0;

	case TOPIC:
		// DW2 only
		pp[0] = Topic();
		return 0;

	case TOPPLAY:
		// Common to both DW1 & DW2
		if (TinselV2) {
			pp -= 3;			// 4 parameters
			TopPlay(coroParam, pp[0], pp[1], pp[2], pp[3], pic->myEscape, pic->event);
			return -4;
		} else {
			pp -= 5;			// 6 parameters
			TopPlay(coroParam, pp[0], pp[1], pp[2], pp[5], pic->idActor, false, 0, pic->escOn, pic->myEscape);
			return -6;
		}

	case TOPWINDOW:
		// Common to both DW1 & DW2
		TopWindow(pp[0]);
		return -1;

	case TRANSLUCENTINDEX:
		// DW2 only
		TranslucentIndex(pp[0]);
		return -1;

	case TRYPLAYSAMPLE:
		// DW1 only
		pp -= 1;			// 2 parameters
		TryPlaySample(coroParam, pp[0], pp[1], pic->escOn, pic->myEscape);
		return -2;

	case UNDIMMUSIC:
		// DW2 only
		UnDimMusic();
		return 0;

	case UNHOOKSCENE:
		// Common to both DW1 & DW2
		UnHookSceneFn();
		return 0;

	case UNTAGACTOR:
		// DW1 only
		UnTagActorFn(pp[0]);
		return -1;

	case VIBRATE:
		// DW1 only
		Vibrate();
		return 0;

	case WAITFRAME:
		// Common to both DW1 & DW2
		pp -= 1;			// 2 parameters
		WaitFrame(coroParam, pp[0], pp[1], pic->escOn, pic->myEscape);
		return -2;

	case WAITKEY:
		// Common to both DW1 & DW2
		WaitKey(coroParam, pic->escOn, pic->myEscape);
		return 0;

	case WAITSCROLL:
		// DW2 only
		WaitScroll(coroParam, pic->myEscape);
		return 0;

	case WAITTIME:
		// Common to both DW1 & DW2
		pp -= 1;			// 2 parameters
		WaitTime(coroParam, pp[0], pp[1], pic->escOn, pic->myEscape);
		if (!coroParam && (pic->hCode == 0x3007540) && (pic->resumeState == RES_2))
			// FIXME: This is a hack to return control to the user after using the prunes in
			// the DW1 Demo, since I can't figure out how it's legitimately done
			Control(CONTROL_ON);

		return -2;

	case WALK:
		// Common to both DW1 & DW2
		pp -= 4;			// 5 parameters
		Walk(coroParam, pp[0], pp[1], pp[2], pp[3], pp[4], false, -1, pic->escOn, pic->myEscape);
		return -5;

	case WALKED: {
		// Common to both DW1 & DW2
		pp -= 3;			// 4 parameters
		bool tmp = false;
		Walked(coroParam, pp[0], pp[1], pp[2], pp[3], pic->escOn, pic->myEscape, tmp);
		if (!coroParam) {
			// Only write the result to the stack if walked actually completed running.
			pp[0] = tmp;
		}
		}
		return -3;

	case WALKINGACTOR:
		// Common to both DW1 & DW2
		if (TinselV2) {
			// DW2 doesn't use a second parameter to WalkingActor
			WalkingActor(pp[0]);
			return -1;
		} else {
			pp -= 40;			// 41 parameters
			WalkingActor(pp[0], (SCNHANDLE *)&pp[1]);
			return -41;
		}

	case WALKPOLY:
		// Common to both DW1 & DW2
		if (TinselV2) {
			pp -= 1;			// 2 parameters
			WalkPoly(coroParam, pp[0], pp[1], pic->hPoly, pic->escOn, pic->myEscape);
			return -2;
		} else {
			pp -= 2;			// 3 parameters
			WalkPoly(coroParam, pp[0], pp[1], pic->hPoly, pic->escOn, pic->myEscape);
			return -3;
		}

	case WALKTAG:
		// Common to both DW1 & DW2
		if (TinselV2) {
			pp -= 1;			// 2 parameters
			WalkTag(coroParam, pp[0], pp[1], pic->hPoly, pic->escOn, pic->myEscape);
			return -2;
		} else {
			pp -= 2;			// 3 parameters
			WalkTag(coroParam, pp[0], pp[1], pic->hPoly, pic->escOn, pic->myEscape);
			return -3;
		}

	case WALKXPOS:
		// DW2 only
		pp[0] = WalkXPos();
		return 0;

	case WALKYPOS:
		// DW2 only
		pp[0] = WalkYPos();
		return 0;

	case WHICHCD:
		// DW2 only
		pp[0] = WhichCd();
		return 0;

	case WHICHINVENTORY:
		// Common to both DW1 & DW2
		pp[0] = WhichInventory();
		return 0;

	case ZZZZZZ:
		// DW2 only - dummy routine used during debugging
		return -1;

	default:
		error("Unsupported library function");
	}

	//error("Can't possibly get here");
}

} // End of namespace Tinsel
