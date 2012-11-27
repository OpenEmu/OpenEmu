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
 * Main purpose is to process user events.
 * Also provides a couple of utility functions.
 */

#include "common/coroutines.h"
#include "tinsel/actors.h"
#include "tinsel/background.h"
#include "tinsel/config.h"
#include "tinsel/cursor.h"
#include "tinsel/dw.h"
#include "tinsel/events.h"
#include "tinsel/handle.h"	// For LockMem()
#include "tinsel/dialogs.h"
#include "tinsel/move.h"	// For walking lead actor
#include "tinsel/pcode.h"	// For Interpret()
#include "tinsel/pdisplay.h"
#include "tinsel/pid.h"
#include "tinsel/polygons.h"
#include "tinsel/rince.h"	// For walking lead actor
#include "tinsel/sched.h"
#include "tinsel/scroll.h"	// For DontScrollCursor()
#include "tinsel/timers.h"	// DwGetCurrentTime()
#include "tinsel/tinlib.h"	// For control()
#include "tinsel/tinsel.h"
#include "tinsel/token.h"

namespace Tinsel {

//----------------- EXTERNAL FUNCTIONS ---------------------

// in PDISPLAY.C
extern int GetTaggedActor();
extern HPOLYGON GetTaggedPoly();

//----------------- EXTERNAL GLOBAL DATA ---------------------

extern bool g_bEnableMenu;

//----------------- LOCAL GLOBAL DATA --------------------

// FIXME: Avoid non-const global vars

static uint32 g_lastUserEvent = 0;	// Time it hapenned
static int g_leftEvents = 0;		// Single or double, left or right. Or escape key.
static int g_escEvents = 1;		// Escape key
static int g_userEvents = 0;		// Whenever a button or a key comes in

static int g_eCount = 0;

static int g_controlState;
static bool g_bStartOff;

static int g_controlX, g_controlY;
static bool g_bProvNotProcessed = false;

/**
 * Gets called before each schedule, only 1 user action per schedule
 * is allowed.
 */
void ResetEcount() {
	g_eCount = 0;
}


void IncUserEvents() {
	g_userEvents++;
	g_lastUserEvent = DwGetCurrentTime();
}

/**
 * If this is a single click, wait to check it's not the first half of a
 * double click.
 * If this is a double click, the process from the waiting single click
 * gets killed.
 */
void AllowDclick(CORO_PARAM, PLR_EVENT be) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	if (be == PLR_SLEFT) {
		GetToken(TOKEN_LEFT_BUT);
		CORO_SLEEP(_vm->_config->_dclickSpeed+1);
		FreeToken(TOKEN_LEFT_BUT);

		// Prevent activation of 2 events on the same tick
		if (++g_eCount != 1)
			CORO_KILL_SELF();

		break;

	} else if (be == PLR_DLEFT) {
		GetToken(TOKEN_LEFT_BUT);
		FreeToken(TOKEN_LEFT_BUT);
	}
	CORO_END_CODE;
}

/**
 * Re-enables user control
 */
void ControlOn() {
	if (!TinselV2) {
		Control(CONTROL_ON);
		return;
	}

	g_bEnableMenu = false;

	if (g_controlState == CONTROL_OFF) {
		// Control is on
		g_controlState = CONTROL_ON;

		// Restore cursor to where it was
		if (g_bStartOff == true)
			g_bStartOff = false;
		else
			SetCursorXY(g_controlX, g_controlY);

		// Re-instate cursor
		UnHideCursor();

		// Turn tags back on
		if (!InventoryActive())
			EnableTags();
	}
}

/**
 * Takes control from the user
 */
void ControlOff() {
	if (!TinselV2) {
		Control(CONTROL_ON);
		return;
	}

	g_bEnableMenu = false;

	if (g_controlState == CONTROL_ON) {
		// Control is off
		g_controlState = CONTROL_OFF;

		// Store cursor position
		GetCursorXY(&g_controlX, &g_controlY, true);

		// Blank out cursor
		DwHideCursor();

		// Switch off tags
		DisableTags();
	}
}

/**
 * Prevent tags and cursor re-appearing
 */
void ControlStartOff() {
	if (!TinselV2) {
		Control(CONTROL_STARTOFF);
		return;
	}

	g_bEnableMenu = false;

	// Control is off
	g_controlState = CONTROL_OFF;

	// Blank out cursor
	DwHideCursor();

	// Switch off tags
	DisableTags();

	g_bStartOff = true;
}

/**
 * Take control from player, if the player has it.
 * Return TRUE if control taken, FALSE if not.
 */
bool GetControl(int param) {
	if (TinselV2)
		return GetControl();

	else if (TestToken(TOKEN_CONTROL)) {
		Control(param);
		return true;
	} else
		return false;
}

bool GetControl() {
	if (g_controlState == CONTROL_ON) {
		ControlOff();
		return true;
	} else
		return false;
}

bool ControlIsOn() {
	if (TinselV2)
		return (g_controlState == CONTROL_ON);

	return TestToken(TOKEN_CONTROL);
}

//-----------------------------------------------------------------------

struct WP_INIT {
	int	x;	// } Where to walk to
	int	y;	// }
};

/**
 * Perform a walk directly initiated by a click.
 */
static void WalkProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		PMOVER pMover;
		int thisWalk;
	CORO_END_CONTEXT(_ctx);

	const WP_INIT *to = (const WP_INIT *)param;	// get the co-ordinates - copied to process when it was created

	CORO_BEGIN_CODE(_ctx);

	_ctx->pMover = GetMover(LEAD_ACTOR);

	if (TinselV2 && MoverIs(_ctx->pMover) && !MoverIsSWalking(_ctx->pMover)) {
		assert(_ctx->pMover->hCpath != NOPOLY); // Lead actor is not in a path

		_ctx->thisWalk = SetActorDest(_ctx->pMover, to->x, to->y, false, 0);
		DontScrollCursor();

		while (MoverMoving(_ctx->pMover) && (_ctx->thisWalk == GetWalkNumber(_ctx->pMover)))
			CORO_SLEEP(1);

	} else if (!TinselV2 && _ctx->pMover->bActive) {
		assert(_ctx->pMover->hCpath != NOPOLY); // Lead actor is not in a path

		GetToken(TOKEN_LEAD);
		SetActorDest(_ctx->pMover, to->x, to->y, false, 0);
		DontScrollCursor();

		while (MoverMoving(_ctx->pMover))
			CORO_SLEEP(1);

		FreeToken(TOKEN_LEAD);
	}

	CORO_END_CODE;
}

void WalkTo(int x, int y) {
	WP_INIT to = { x, y };

	CoroScheduler.createProcess(PID_TCODE, WalkProcess, &to, sizeof(to));
}

/**
 * Run appropriate actor or polygon glitter code.
 * If none, and it's a WALKTO event, do a walk.
 */
static void ProcessUserEvent(TINSEL_EVENT uEvent, const Common::Point &coOrds, PLR_EVENT be = PLR_NOEVENT) {
	int	actor;
	int	aniX, aniY;
	HPOLYGON hPoly;

	// Prevent activation of 2 events on the same tick
	if (++g_eCount != 1)
		return;

	if ((actor = GetTaggedActor()) != 0) {
		// Event for a tagged actor
		if (TinselV2)
			ActorEvent(Common::nullContext, actor, uEvent, false, 0);
		else
			ActorEvent(actor, uEvent, be);
	} else if ((hPoly = GetTaggedPoly()) != NOPOLY) {
		// Event for active tagged polygon
		if (!TinselV2)
			RunPolyTinselCode(hPoly, uEvent, be, false);
		else if (uEvent != PROV_WALKTO)
			PolygonEvent(Common::nullContext, hPoly, uEvent, 0, false, 0);

	} else {
		GetCursorXY(&aniX, &aniY, true);

		// There could be a poly involved which has no tag.
		if ((hPoly = InPolygon(aniX, aniY, TAG)) != NOPOLY ||
			(!TinselV2 && ((hPoly = InPolygon(aniX, aniY, EXIT)) != NOPOLY))) {
			if (TinselV2 && (uEvent != PROV_WALKTO))
				PolygonEvent(Common::nullContext, hPoly, uEvent, 0, false, 0);
			else if (!TinselV2)
				RunPolyTinselCode(hPoly, uEvent, be, false);
		} else if ((uEvent == PROV_WALKTO) || (uEvent == WALKTO)) {
			if (TinselV2)
				ProcessedProvisional();
			WalkTo(aniX, aniY);
		}
	}
}


/**
 * ProcessButEvent
 */
void ProcessButEvent(PLR_EVENT be) {
	if (_vm->_config->_swapButtons) {
		switch (be) {
		case PLR_SLEFT:
			be = PLR_SRIGHT;
			break;
		case PLR_DLEFT:
			be = PLR_DRIGHT;
			break;
		case PLR_SRIGHT:
			be = PLR_SLEFT;
			break;
		case PLR_DRIGHT:
			be = PLR_DLEFT;
			break;
		case PLR_DRAG1_START:
			be = PLR_DRAG2_START;
			break;
		case PLR_DRAG1_END:
			be = PLR_DRAG2_END;
			break;
		case PLR_DRAG2_START:
			be = PLR_DRAG1_START;
			break;
		case PLR_DRAG2_END:
			be = PLR_DRAG1_END;
			break;
		default:
			break;
		}
	}

	PlayerEvent(be, _vm->getMousePosition());
}

/**
 * ProcessKeyEvent
 */

void ProcessKeyEvent(PLR_EVENT ke) {
	// Pass the keyboard event to the player event handler
	int xp, yp;
	GetCursorXYNoWait(&xp, &yp, true);
	const Common::Point mousePos(xp, yp);

	PlayerEvent(ke, mousePos);
}

#define REAL_ACTION_CHECK if (TinselV2) { \
	if (DwGetCurrentTime() - lastRealAction < 4) return; \
	lastRealAction = DwGetCurrentTime(); \
}

/**
 * Main interface point for specifying player atcions
 */
void PlayerEvent(PLR_EVENT pEvent, const Common::Point &coOrds) {
	// Logging of player actions
	const char *actionList[] = {
		"PLR_PROV_WALKTO", "PLR_WALKTO", "PLR_LOOK", "PLR_ACTION", "PLR_ESCAPE",
		"PLR_MENU", "PLR_QUIT", "PLR_PGUP", "PLR_PGDN", "PLR_HOME", "PLR_END",
		"PLR_DRAG1_START", "PLR_DRAG1_END", "PLR_DRAG2_START", "PLR_DRAG2_END",
		"PLR_JUMP", "PLR_NOEVENT"};
	debugC(DEBUG_BASIC, kTinselDebugActions, "%s - (%d,%d)",
		actionList[pEvent], coOrds.x, coOrds.y);
	static uint32 lastRealAction = 0;	// FIXME: Avoid non-const global vars

	// This stuff to allow F1 key during startup.
	if (g_bEnableMenu && pEvent == PLR_MENU)
		Control(CONTROL_ON);
	else
		IncUserEvents();

	if (pEvent == PLR_ESCAPE) {
		++g_escEvents;
		++g_leftEvents;		// Yes, I do mean this
	} else if ((pEvent == PLR_PROV_WALKTO)
			|| (pEvent == PLR_WALKTO)
			|| (pEvent == PLR_LOOK)
			|| (pEvent == PLR_ACTION)) {
		++g_leftEvents;
	}

	// Only allow events if player control is on
	if (!ControlIsOn() && (pEvent != PLR_DRAG1_END))
		return;

	if (TinselV2 && InventoryActive()) {
		int x, y;
		PlayfieldGetPos(FIELD_WORLD, &x, &y);
		EventToInventory(pEvent, Common::Point(coOrds.x - x, coOrds.y - y));
		return;
	}

	switch (pEvent) {
	case PLR_QUIT:
		OpenMenu(QUIT_MENU);
		break;

	case PLR_MENU:
		OpenMenu(MAIN_MENU);
		break;

	case PLR_JUMP:
		OpenMenu(HOPPER_MENU1);
		break;

	case PLR_SAVE:
		OpenMenu(SAVE_MENU);
		break;

	case PLR_LOAD:
		OpenMenu(LOAD_MENU);
		break;

	case PLR_PROV_WALKTO:		// Provisional WALKTO !
		ProcessUserEvent(PROV_WALKTO, coOrds);
		break;

	case PLR_WALKTO:
		REAL_ACTION_CHECK;

		if (TinselV2 || !InventoryActive())
			ProcessUserEvent(WALKTO, coOrds, PLR_SLEFT);
		else
			EventToInventory(PLR_SLEFT, coOrds);
		break;

	case PLR_ACTION:
		REAL_ACTION_CHECK;

		if (TinselV2 || !InventoryActive())
			ProcessUserEvent(ACTION, coOrds, PLR_DLEFT);
		else
			EventToInventory(PLR_DLEFT, coOrds);
		break;

	case PLR_LOOK:
		REAL_ACTION_CHECK;

		if (TinselV2 || !InventoryActive())
			ProcessUserEvent(LOOK, coOrds, PLR_SRIGHT);
		else
			EventToInventory(PLR_SRIGHT, coOrds);
		break;

	default:
		if (InventoryActive())
			EventToInventory(pEvent, coOrds);
		break;
	}
}

/**
 * For ESCapable Glitter sequences
 */
int GetEscEvents() {
	return g_escEvents;
}

/**
 * For cutting short talk()s etc.
 */
int GetLeftEvents() {
	return g_leftEvents;
}

bool LeftEventChange(int myleftEvent) {
	if (g_leftEvents != myleftEvent) {
		ProcessedProvisional();
		return true;
	} else
		return false;
}

/**
 * For waitkey() Glitter function
 */
int getUserEvents() {
	return g_userEvents;
}

uint32 getUserEventTime() {
	return DwGetCurrentTime() - g_lastUserEvent;
}

void resetUserEventTime() {
	g_lastUserEvent = DwGetCurrentTime();
}

struct PTP_INIT {
	HPOLYGON	hPoly;		// Polygon
	TINSEL_EVENT	event;		// Trigerring event
	PLR_EVENT	bev;		// To allow for double clicks
	bool		take_control;	// Set if control should be taken
					// while code is running.
	int		actor;

	PINT_CONTEXT	pic;
};

/**
 * Runs glitter code associated with a polygon.
 */
void PolyTinselProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		INT_CONTEXT *pic;
		bool bTookControl;	// Set if this function takes control

	CORO_END_CONTEXT(_ctx);

	const PTP_INIT *to = (const PTP_INIT *)param;	// get the stuff copied to process when it was created

	CORO_BEGIN_CODE(_ctx);

	if (TinselV2) {

		// Take control for CONVERSE events
		if (to->event == CONVERSE) {
			_ctx->bTookControl = GetControl();
			HideConversation(true);
		} else
			_ctx->bTookControl = false;

		CORO_INVOKE_1(Interpret, to->pic);

		// Restore conv window if applicable
		if (to->event == CONVERSE) {
			// Free control if we took it
			if (_ctx->bTookControl)
				ControlOn();

			HideConversation(false);
		}

	} else {

		CORO_INVOKE_1(AllowDclick, to->bev);	// May kill us if single click

		// Control may have gone off during AllowDclick()
		if (!TestToken(TOKEN_CONTROL)
			&& (to->event == WALKTO || to->event == ACTION || to->event == LOOK))
			CORO_KILL_SELF();

		// Take control, if requested
		if (to->take_control)
			_ctx->bTookControl = GetControl(CONTROL_OFF);
		else
			_ctx->bTookControl = false;

		// Hide conversation if appropriate
		if (to->event == CONVERSE)
			HideConversation(true);

		// Run the code
		_ctx->pic = InitInterpretContext(GS_POLYGON, GetPolyScript(to->hPoly), to->event, to->hPoly, to->actor, NULL);
		CORO_INVOKE_1(Interpret, _ctx->pic);

		// Free control if we took it
		if (_ctx->bTookControl)
			Control(CONTROL_ON);

		// Restore conv window if applicable
		if (to->event == CONVERSE)
			HideConversation(false);
	}

	CORO_END_CODE;
}

/**
 * Run the Polygon process with the given event
 */
void PolygonEvent(CORO_PARAM, HPOLYGON hPoly, TINSEL_EVENT tEvent, int actor, bool bWait,
				  int myEscape, bool *result) {
	CORO_BEGIN_CONTEXT;
		Common::PPROCESS pProc;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	PTP_INIT to;

	if (result)
		*result = false;
	to.hPoly = -1;
	to.event = tEvent;
	to.pic = InitInterpretContext(GS_POLYGON,
			GetPolyScript(hPoly),
			tEvent,
			hPoly,			// Polygon
			actor,			// Actor
			NULL,			// No Object
			myEscape);
	if (to.pic != NULL) {
		_ctx->pProc = CoroScheduler.createProcess(PID_TCODE, PolyTinselProcess, &to, sizeof(to));
		AttachInterpret(to.pic, _ctx->pProc);

		if (bWait)
			CORO_INVOKE_2(WaitInterpret, _ctx->pProc, result);
	}

	CORO_END_CODE;
}

/**
 * Runs glitter code associated with a polygon.
 */
void RunPolyTinselCode(HPOLYGON hPoly, TINSEL_EVENT event, PLR_EVENT be, bool tc) {
	PTP_INIT to = { hPoly, event, be, tc, 0, NULL };

	assert(!TinselV2);
	CoroScheduler.createProcess(PID_TCODE, PolyTinselProcess, &to, sizeof(to));
}

void effRunPolyTinselCode(HPOLYGON hPoly, TINSEL_EVENT event, int actor) {
	PTP_INIT to = { hPoly, event, PLR_NOEVENT, false, actor, NULL };

	assert(!TinselV2);
	CoroScheduler.createProcess(PID_TCODE, PolyTinselProcess, &to, sizeof(to));
}

/**
 *  If provisional event was processed, calling this prevents the
 * subsequent 'real' event.
 */
void ProcessedProvisional() {
	g_bProvNotProcessed = false;
}

/**
 * Resets the bProvNotProcessed flag
 */
void ProvNotProcessed() {
	g_bProvNotProcessed = true;
}

bool GetProvNotProcessed() {
	return g_bProvNotProcessed;
}

} // End of namespace Tinsel
