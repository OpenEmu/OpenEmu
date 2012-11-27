/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "common/system.h"
#include "common/events.h"
#include "common/memstream.h"
#include "common/textconsole.h"

#include "graphics/cursorman.h"

#include "sword2/sword2.h"
#include "sword2/console.h"
#include "sword2/controls.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/mouse.h"
#include "sword2/object.h"
#include "sword2/resman.h"
#include "sword2/screen.h"
#include "sword2/sound.h"

namespace Sword2 {

// Pointer resource id's

enum {
	CROSHAIR	= 18,
	EXIT0		= 788,
	EXIT1		= 789,
	EXIT2		= 790,
	EXIT3		= 791,
	EXIT4		= 792,
	EXIT5		= 793,
	EXIT6		= 794,
	EXIT7		= 795,
	EXITDOWN	= 796,
	EXITUP		= 797,
	MOUTH		= 787,
	NORMAL		= 17,
	PICKUP		= 3099,
	SCROLL_L	= 1440,
	SCROLL_R	= 1441,
	USE		= 3100
};

Mouse::Mouse(Sword2Engine *vm) {
	_vm = vm;

	resetMouseList();

	_mouseTouching = 0;
	_oldMouseTouching = 0;
	_menuSelectedPos = 0;
	_examiningMenuIcon = false;
	_mousePointerRes = 0;
	_mouseMode = 0;
	_mouseStatus = false;
	_mouseModeLocked = false;
	_currentLuggageResource = 0;
	_oldButton = 0;
	_buttonClick = 0;
	_pointerTextBlocNo = 0;
	_playerActivityDelay = 0;
	_realLuggageItem = 0;

	_mouseAnim.data = NULL;
	_luggageAnim.data = NULL;

	// For the menus
	_totalTemp = 0;
	memset(_tempList, 0, sizeof(_tempList));

	_totalMasters = 0;
	memset(_masterMenuList, 0, sizeof(_masterMenuList));
	memset(_mouseList, 0, sizeof(_mouseList));
	memset(_subjectList, 0, sizeof(_subjectList));

	_defaultResponseId = 0;
	_choosing = false;

	_iconCount = 0;

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < RDMENU_MAXPOCKETS; j++) {
			_icons[i][j] = NULL;
			_pocketStatus[i][j] = 0;
		}

		_menuStatus[i] = RDMENU_HIDDEN;
	}
}

Mouse::~Mouse() {
	free(_mouseAnim.data);
	free(_luggageAnim.data);
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < RDMENU_MAXPOCKETS; j++)
			free(_icons[i][j]);
}

void Mouse::getPos(int &x, int &y) {
	Common::EventManager *eventMan = _vm->_system->getEventManager();
	Common::Point pos = eventMan->getMousePos();

	x = pos.x;
	y = pos.y - MENUDEEP;
}

int Mouse::getX() {
	int x, y;

	getPos(x, y);
	return x;
}

int Mouse::getY() {
	int x, y;

	getPos(x, y);
	return y;
}

/**
 * Call at beginning of game loop
 */

void Mouse::resetMouseList() {
	_curMouse = 0;
}

void Mouse::registerMouse(byte *ob_mouse, BuildUnit *build_unit) {
	assert(_curMouse < TOTAL_mouse_list);

	ObjectMouse mouse;

	mouse.read(ob_mouse);

	if (!mouse.pointer)
		return;

	if (build_unit) {
		_mouseList[_curMouse].rect.left = build_unit->x;
		_mouseList[_curMouse].rect.top = build_unit->y;
		_mouseList[_curMouse].rect.right = 1 + build_unit->x + build_unit->scaled_width;
		_mouseList[_curMouse].rect.bottom = 1 + build_unit->y + build_unit->scaled_height;
	} else {
		_mouseList[_curMouse].rect.left = mouse.x1;
		_mouseList[_curMouse].rect.top = mouse.y1;
		_mouseList[_curMouse].rect.right = 1 + mouse.x2;
		_mouseList[_curMouse].rect.bottom = 1 + mouse.y2;
	}

	_mouseList[_curMouse].priority = mouse.priority;
	_mouseList[_curMouse].pointer = mouse.pointer;

	// Change all COGS pointers to CROSHAIR. I'm guessing that this was a
	// design decision made in mid-development and they didn't want to go
	// back and re-generate the resource files.

	if (_mouseList[_curMouse].pointer == USE)
		_mouseList[_curMouse].pointer = CROSHAIR;

	// Check if pointer text field is set due to previous object using this
	// slot (ie. not correct for this one)

	// If 'pointer_text' field is set, but the 'id' field isn't same is
	// current id then we don't want this "left over" pointer text

	if (_mouseList[_curMouse].pointer_text && _mouseList[_curMouse].id != (int32)_vm->_logic->readVar(ID))
		_mouseList[_curMouse].pointer_text = 0;

	// Get id from system variable 'id' which is correct for current object
	_mouseList[_curMouse].id = _vm->_logic->readVar(ID);

	_curMouse++;
}

void Mouse::registerPointerText(int32 text_id) {
	assert(_curMouse < TOTAL_mouse_list);

	// current object id - used for checking pointer_text when mouse area
	// registered (in fnRegisterMouse and fnRegisterFrame)

	_mouseList[_curMouse].id = _vm->_logic->readVar(ID);
	_mouseList[_curMouse].pointer_text = text_id;
}

/**
 * This function is called every game cycle.
 */

void Mouse::mouseEngine() {
	monitorPlayerActivity();
	clearPointerText();

	// If George is dead, the system menu is visible all the time, and is
	// the only thing that can be used.

	if (_vm->_logic->readVar(DEAD)) {
		if (_mouseMode != MOUSE_system_menu) {
			_mouseMode = MOUSE_system_menu;

			if (_mouseTouching) {
				_oldMouseTouching = 0;
				_mouseTouching = 0;
			}

			setMouse(NORMAL_MOUSE_ID);
			buildSystemMenu();
		}
		systemMenuMouse();
		return;
	}

	// If the mouse is not visible, do nothing

	if (_mouseStatus)
		return;

	switch (_mouseMode) {
	case MOUSE_normal:
		normalMouse();
		break;
	case MOUSE_menu:
		menuMouse();
		break;
	case MOUSE_drag:
		dragMouse();
		break;
	case MOUSE_system_menu:
		systemMenuMouse();
		break;
	case MOUSE_holding:
		if (getY() < 400) {
			_mouseMode = MOUSE_normal;
			debug(5, "   releasing");
		}
		break;
	default:
		break;
	}
}

#if RIGHT_CLICK_CLEARS_LUGGAGE
bool Mouse::heldIsInInventory() {
	int32 object_held = (int32)_vm->_logic->readVar(OBJECT_HELD);

	for (uint i = 0; i < _totalMasters; i++) {
		if (_masterMenuList[i].icon_resource == object_held)
			return true;
	}
	return false;
}
#endif

int Mouse::menuClick(int menu_items) {
	int x = getX();
	byte menuIconWidth;

	if (Sword2Engine::isPsx())
		menuIconWidth = RDMENU_PSXICONWIDE;
	else
		menuIconWidth = RDMENU_ICONWIDE;


	if (x < RDMENU_ICONSTART)
		return -1;

	if (x > RDMENU_ICONSTART + menu_items * (menuIconWidth + RDMENU_ICONSPACING) - RDMENU_ICONSPACING)
		return -1;

	return (x - RDMENU_ICONSTART) / (menuIconWidth + RDMENU_ICONSPACING);
}

void Mouse::systemMenuMouse() {
	uint32 safe_looping_music_id;
	MouseEvent *me;
	int hit;
	byte *icon;
	int32 pars[2];
	uint32 icon_list[5] = {
		OPTIONS_ICON,
		QUIT_ICON,
		SAVE_ICON,
		RESTORE_ICON,
		RESTART_ICON
	};

	// If the mouse is moved off the menu, close it. Unless the player is
	// dead, in which case the menu should always be visible.

	int y = getY();

	if (y > 0 && !_vm->_logic->readVar(DEAD)) {
		_mouseMode = MOUSE_normal;
		hideMenu(RDMENU_TOP);
		return;
	}

	// Check if the user left-clicks anywhere in the menu area.

	me = _vm->mouseEvent();

	if (!me || !(me->buttons & RD_LEFTBUTTONDOWN))
		return;

	if (y > 0)
		return;

	hit = menuClick(ARRAYSIZE(icon_list));

	if (hit < 0)
		return;

	// Do nothing if using PSX version and are on TOP menu.

	if ((icon_list[hit] == OPTIONS_ICON || icon_list[hit] == QUIT_ICON
		|| icon_list[hit] == SAVE_ICON || icon_list[hit] == RESTORE_ICON
		|| icon_list[hit] == RESTART_ICON) && Sword2Engine::isPsx())
		return;

	// No save when dead

	if (icon_list[hit] == SAVE_ICON && _vm->_logic->readVar(DEAD))
		return;

	// Gray out all he icons, except the one that was clicked

	for (int i = 0; i < ARRAYSIZE(icon_list); i++) {
		if (i != hit) {
			icon = _vm->_resman->openResource(icon_list[i]) + ResHeader::size();
			setMenuIcon(RDMENU_TOP, i, icon);
			_vm->_resman->closeResource(icon_list[i]);
		}
	}

	_vm->_sound->pauseFx();

	// NB. Need to keep a safe copy of '_loopingMusicId' for savegame & for
	// playing when returning from control panels because control panel
	// music will overwrite it!

	safe_looping_music_id = _vm->_sound->getLoopingMusicId();

	pars[0] = 221;
	pars[1] = FX_LOOP;
	_vm->_logic->fnPlayMusic(pars);

	// HACK: Restore proper looping_music_id
	_vm->_sound->setLoopingMusicId(safe_looping_music_id);

	processMenu();

	// call the relevant screen

	switch (hit) {
	case 0:
		{
			OptionsDialog dialog(_vm);
			dialog.runModal();
		}
		break;
	case 1:
		{
			QuitDialog dialog(_vm);
			dialog.runModal();
		}
		break;
	case 2:
		{
			SaveDialog dialog(_vm);
			dialog.runModal();
		}
		break;
	case 3:
		{
			RestoreDialog dialog(_vm);
			dialog.runModal();
		}
		break;
	case 4:
		{
			RestartDialog dialog(_vm);
			dialog.runModal();
		}
		break;
	}

	// Menu stays open on death screen. Otherwise it's closed.

	if (!_vm->_logic->readVar(DEAD)) {
		_mouseMode = MOUSE_normal;
		hideMenu(RDMENU_TOP);
	} else {
		setMouse(NORMAL_MOUSE_ID);
		buildSystemMenu();
	}

	// Back to the game again

	processMenu();

	// Reset game palette, but not after a successful restore or restart!
	// See RestoreFromBuffer() in saveload.cpp

	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();

	if (screenInfo->new_palette != 99) {
		// 0 means put back game screen palette; see build_display.cpp
		_vm->_screen->setFullPalette(0);

		// Stop the engine fading in the restored screens palette
		screenInfo->new_palette = 0;
	} else
		screenInfo->new_palette = 1;

	_vm->_sound->unpauseFx();

	// If there was looping music before coming into the control panels
	// then restart it! NB. If a game has been restored the music will be
	// restarted twice, but this shouldn't cause any harm.

	if (_vm->_sound->getLoopingMusicId()) {
		pars[0] = _vm->_sound->getLoopingMusicId();
		pars[1] = FX_LOOP;
		_vm->_logic->fnPlayMusic(pars);
	} else
		_vm->_logic->fnStopMusic(NULL);
}

void Mouse::dragMouse() {
	byte buf1[NAME_LEN], buf2[NAME_LEN];
	MouseEvent *me;
	int hit;

	// We can use dragged object both on other inventory objects, or on
	// objects in the scene, so if the mouse moves off the inventory menu,
	// then close it.

	int x, y;

	getPos(x, y);

	if (y < 400) {
		_mouseMode = MOUSE_normal;
		hideMenu(RDMENU_BOTTOM);
		return;
	}

	// Handles cursors and the luggage on/off according to type

	mouseOnOff();

	// Now do the normal click stuff

	me = _vm->mouseEvent();

	if (!me)
		return;

#if RIGHT_CLICK_CLEARS_LUGGAGE
	if ((me->buttons & RD_RIGHTBUTTONDOWN) && heldIsInInventory()) {
		_vm->_logic->writeVar(OBJECT_HELD, 0);
		_menuSelectedPos = 0;
		_mouseMode = MOUSE_menu;
		setLuggage(0);
		buildMenu();
		return;
	}
#endif

	if (!(me->buttons & RD_LEFTBUTTONDOWN))
		return;

	// there's a mouse event to be processed

	// could be clicking on an on screen object or on the menu
	// which is currently displayed

	if (_mouseTouching) {
		// mouse is over an on screen object - and we have luggage

		// Depending on type we'll maybe kill the object_held - like
		// for exits

		// Set global script variable 'button'. We know that it was the
		// left button, not the right one.

		_vm->_logic->writeVar(LEFT_BUTTON, 1);
		_vm->_logic->writeVar(RIGHT_BUTTON, 0);

		// These might be required by the action script about to be run
		ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();

		_vm->_logic->writeVar(MOUSE_X, x + screenInfo->scroll_offset_x);
		_vm->_logic->writeVar(MOUSE_Y, y + screenInfo->scroll_offset_y);

		// For scripts to know what's been clicked. First used for
		// 'room_13_turning_script' in object 'biscuits_13'

		_vm->_logic->writeVar(CLICKED_ID, _mouseTouching);

		_vm->_logic->setPlayerActionEvent(CUR_PLAYER_ID, _mouseTouching);

		debug(2, "Used \"%s\" on \"%s\"",
			_vm->_resman->fetchName(_vm->_logic->readVar(OBJECT_HELD), buf1),
			_vm->_resman->fetchName(_vm->_logic->readVar(CLICKED_ID), buf2));

		// Hide menu - back to normal menu mode

		hideMenu(RDMENU_BOTTOM);
		_mouseMode = MOUSE_normal;

		return;
	}

	// Better check for combine/cancel. Cancel puts us back in MOUSE_menu
	// mode

	hit = menuClick(TOTAL_engine_pockets);

	if (hit < 0 || !_masterMenuList[hit].icon_resource)
		return;

	// Always back into menu mode. Remove the luggage as well.

	_mouseMode = MOUSE_menu;
	setLuggage(0);

	if ((uint)hit == _menuSelectedPos) {
		// If we clicked on the same icon again, reset the first icon

		_vm->_logic->writeVar(OBJECT_HELD, 0);
		_menuSelectedPos = 0;
	} else {
		// Otherwise, combine the two icons

		_vm->_logic->writeVar(COMBINE_BASE, _masterMenuList[hit].icon_resource);
		_vm->_logic->setPlayerActionEvent(CUR_PLAYER_ID, MENU_MASTER_OBJECT);

		// Turn off mouse now, to prevent player trying to click
		// elsewhere BUT leave the bottom menu open

		hideMouse();

		debug(2, "Used \"%s\" on \"%s\"",
			_vm->_resman->fetchName(_vm->_logic->readVar(OBJECT_HELD), buf1),
			_vm->_resman->fetchName(_vm->_logic->readVar(COMBINE_BASE), buf2));
	}

	// Refresh the menu

	buildMenu();
}

void Mouse::menuMouse() {
	MouseEvent *me;
	int hit;

	// If the mouse is moved off the menu, close it.

	if (getY() < 400) {
		_mouseMode = MOUSE_normal;
		hideMenu(RDMENU_BOTTOM);
		return;
	}

	me = _vm->mouseEvent();

	if (!me)
		return;

	hit = menuClick(TOTAL_engine_pockets);

	// Check if we clicked on an actual icon.

	if (hit < 0 || !_masterMenuList[hit].icon_resource)
		return;

	if (me->buttons & RD_RIGHTBUTTONDOWN) {
		// Right button - examine an object, identified by its icon
		// resource id.

		_examiningMenuIcon = true;
		_vm->_logic->writeVar(OBJECT_HELD, _masterMenuList[hit].icon_resource);

		// Must clear this so next click on exit becomes 1st click
		// again

		_vm->_logic->writeVar(EXIT_CLICK_ID, 0);

		_vm->_logic->setPlayerActionEvent(CUR_PLAYER_ID, MENU_MASTER_OBJECT);

		// Refresh the menu

		buildMenu();

		// Turn off mouse now, to prevent player trying to click
		// elsewhere BUT leave the bottom menu open

		hideMouse();

		debug(2, "Right-click on \"%s\" icon",
			_vm->_resman->fetchName(_vm->_logic->readVar(OBJECT_HELD)));

		return;
	}

	if (me->buttons & RD_LEFTBUTTONDOWN) {
		// Left button - bung us into drag luggage mode. The object is
		// identified by its icon resource id. We need the luggage
		// resource id for mouseOnOff

		_mouseMode = MOUSE_drag;

		_menuSelectedPos = hit;
		_vm->_logic->writeVar(OBJECT_HELD, _masterMenuList[hit].icon_resource);
		_currentLuggageResource = _masterMenuList[hit].luggage_resource;

		// Must clear this so next click on exit becomes 1st click
		// again

		_vm->_logic->writeVar(EXIT_CLICK_ID, 0);

		// Refresh the menu

		buildMenu();

		setLuggage(_masterMenuList[hit].luggage_resource);

		debug(2, "Left-clicked on \"%s\" icon - switch to drag mode",
			_vm->_resman->fetchName(_vm->_logic->readVar(OBJECT_HELD)));
	}
}

void Mouse::normalMouse() {
	// The game is playing and none of the menus are activated - but, we
	// need to check if a menu is to start. Note, won't have luggage

	MouseEvent *me;

	// Check if the cursor has moved onto the system menu area. No save in
	// big-object menu lock situation, of if the player is dragging an
	// object.

	int x, y;

	getPos(x, y);

	if (y < 0 && !_mouseModeLocked && !_vm->_logic->readVar(OBJECT_HELD)) {
		_mouseMode = MOUSE_system_menu;

		if (_mouseTouching) {
			// We were on something, but not anymore
			_oldMouseTouching = 0;
			_mouseTouching = 0;
		}

		// Reset mouse cursor - in case we're between mice

		setMouse(NORMAL_MOUSE_ID);
		buildSystemMenu();
		return;
	}

	// Check if the cursor has moved onto the inventory menu area. No
	// inventory in big-object menu lock situation,

	if (y > 399 && !_mouseModeLocked) {
		// If an object is being held, i.e. if the mouse cursor has a
		// luggage, go to drag mode instead of menu mode, but the menu
		// is still opened.
		//
		// That way, we can still use an object on another inventory
		// object, even if the inventory menu was closed after the
		// first object was selected.

		if (!_vm->_logic->readVar(OBJECT_HELD))
			_mouseMode = MOUSE_menu;
		else
			_mouseMode = MOUSE_drag;

		// If mouse is moving off an object and onto the menu then do a
		// standard get-off

		if (_mouseTouching) {
			_oldMouseTouching = 0;
			_mouseTouching = 0;
		}

		// Reset mouse cursor

		setMouse(NORMAL_MOUSE_ID);
		buildMenu();
		return;
	}

	// Check for moving the mouse on or off things

	mouseOnOff();

	me = _vm->mouseEvent();

	if (!me)
		return;

	bool button_down = (me->buttons & (RD_LEFTBUTTONDOWN | RD_RIGHTBUTTONDOWN)) != 0;

	// For debugging. We can draw a rectangle on the screen and see its
	// coordinates. This was probably used to help defining hit areas.

	if (_vm->_debugger->_definingRectangles) {
		ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();

		if (_vm->_debugger->_draggingRectangle == 0) {
			// Not yet dragging a rectangle, so need click to start

			if (button_down) {
				// set both (x1,y1) and (x2,y2) to this point
				_vm->_debugger->_rectX1 = _vm->_debugger->_rectX2 = (uint32)x + screenInfo->scroll_offset_x;
				_vm->_debugger->_rectY1 = _vm->_debugger->_rectY2 = (uint32)y + screenInfo->scroll_offset_y;
				_vm->_debugger->_draggingRectangle = 1;
			}
		} else if (_vm->_debugger->_draggingRectangle == 1) {
			// currently dragging a rectangle - click means reset

			if (button_down) {
				// lock rectangle, so you can let go of mouse
				// to type in the coords
				_vm->_debugger->_draggingRectangle = 2;
			} else {
				// drag rectangle
				_vm->_debugger->_rectX2 = (uint32)x + screenInfo->scroll_offset_x;
				_vm->_debugger->_rectY2 = (uint32)y + screenInfo->scroll_offset_y;
			}
		} else {
			// currently locked to avoid knocking out of place
			// while reading off the coords

			if (button_down) {
				// click means reset - back to start again
				_vm->_debugger->_draggingRectangle = 0;
			}
		}

		return;
	}

#if RIGHT_CLICK_CLEARS_LUGGAGE
	if (_vm->_logic->readVar(OBJECT_HELD) && (me->buttons & RD_RIGHTBUTTONDOWN) && heldIsInInventory()) {
		_vm->_logic->writeVar(OBJECT_HELD, 0);
		_menuSelectedPos = 0;
		setLuggage(0);
		return;
	}
#endif

	// Now do the normal click stuff

	// We only care about down clicks when the mouse is over an object.

	if (!_mouseTouching || !button_down)
		return;

	// There's a mouse event to be processed and the mouse is on something.
	// Notice that the floor itself is considered an object.

	// There are no menus about so its nice and simple. This is as close to
	// the old advisor_188 script as we get, I'm sorry to say.

	// If player is walking or relaxing then those need to terminate
	// correctly. Otherwise set player run the targets action script or, do
	// a special walk if clicking on the scroll-more icon

	// PLAYER_ACTION script variable - whatever catches this must reset to
	// 0 again
	// _vm->_logic->writeVar(PLAYER_ACTION, _mouseTouching);

	// Idle or router-anim will catch it

	// Set global script variable 'button'

	if (me->buttons & RD_LEFTBUTTONDOWN) {
		_vm->_logic->writeVar(LEFT_BUTTON, 1);
		_vm->_logic->writeVar(RIGHT_BUTTON, 0);
		_buttonClick = 0;	// for re-click
	} else {
		_vm->_logic->writeVar(LEFT_BUTTON, 0);
		_vm->_logic->writeVar(RIGHT_BUTTON, 1);
		_buttonClick = 1;	// for re-click
	}

	// These might be required by the action script about to be run
	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();

	_vm->_logic->writeVar(MOUSE_X, x + screenInfo->scroll_offset_x);
	_vm->_logic->writeVar(MOUSE_Y, y + screenInfo->scroll_offset_y);

	if (_mouseTouching == _vm->_logic->readVar(EXIT_CLICK_ID) && (me->buttons & RD_LEFTBUTTONDOWN)) {
		// It's the exit double click situation. Let the existing
		// interaction continue and start fading down. Switch the human
		// off too

		noHuman();
		_vm->_logic->fnFadeDown(NULL);

		// Tell the walker

		_vm->_logic->writeVar(EXIT_FADING, 1);
	} else if (_oldButton == _buttonClick && _mouseTouching == _vm->_logic->readVar(CLICKED_ID) && _mousePointerRes != NORMAL_MOUSE_ID) {
		// Re-click. Do nothing, except on floors
	} else {
		// For re-click

		_oldButton = _buttonClick;

		// For scripts to know what's been clicked. First used for
		// 'room_13_turning_script' in object 'biscuits_13'

		_vm->_logic->writeVar(CLICKED_ID, _mouseTouching);

		// Must clear these two double-click control flags - do it here
		// so reclicks after exit clicks are cleared up

		_vm->_logic->writeVar(EXIT_CLICK_ID, 0);
		_vm->_logic->writeVar(EXIT_FADING, 0);

		_vm->_logic->setPlayerActionEvent(CUR_PLAYER_ID, _mouseTouching);

		byte buf1[NAME_LEN], buf2[NAME_LEN];

		if (_vm->_logic->readVar(OBJECT_HELD))
			debug(2, "Used \"%s\" on \"%s\"",
				_vm->_resman->fetchName(_vm->_logic->readVar(OBJECT_HELD), buf1),
				_vm->_resman->fetchName(_vm->_logic->readVar(CLICKED_ID), buf2));
		else if (_vm->_logic->readVar(LEFT_BUTTON))
			debug(2, "Left-clicked on \"%s\"",
				_vm->_resman->fetchName(_vm->_logic->readVar(CLICKED_ID)));
		else	// RIGHT BUTTON
			debug(2, "Right-clicked on \"%s\"",
				_vm->_resman->fetchName(_vm->_logic->readVar(CLICKED_ID)));
	}
}

uint32 Mouse::chooseMouse() {
	// Unlike the other mouse "engines", this one is called directly by the
	// fnChoose() opcode.

	byte menuIconWidth;

	if (Sword2Engine::isPsx())
		menuIconWidth = RDMENU_PSXICONWIDE;
	else
		menuIconWidth = RDMENU_ICONWIDE;


	uint i;

	_vm->_logic->writeVar(AUTO_SELECTED, 0);

	uint32 in_subject = _vm->_logic->readVar(IN_SUBJECT);
	uint32 object_held = _vm->_logic->readVar(OBJECT_HELD);

	if (object_held) {
		// The player used an object on a person. In this case it
		// triggered a conversation menu. Act as if the user tried to
		// talk to the person about that object. If the person doesn't
		// know anything about it, use the default response.

		uint32 response = _defaultResponseId;

		for (i = 0; i < in_subject; i++) {
			if (_subjectList[i].res == object_held) {
				response = _subjectList[i].ref;
				break;
			}
		}

		// The user won't be holding the object any more, and the
		// conversation menu will be closed.

		_vm->_logic->writeVar(OBJECT_HELD, 0);
		_vm->_logic->writeVar(IN_SUBJECT, 0);
		return response;
	}

	if (_vm->_logic->readVar(CHOOSER_COUNT_FLAG) == 0 && in_subject == 1 && _subjectList[0].res == EXIT_ICON) {
		// This is the first time the chooser is coming up in this
		// conversation, there is only one subject and that's the
		// EXIT icon.
		//
		// In other words, the player doesn't have anything to talk
		// about. Skip it.

		// The conversation menu will be closed. We set AUTO_SELECTED
		// because the speech script depends on it.

		_vm->_logic->writeVar(AUTO_SELECTED, 1);
		_vm->_logic->writeVar(IN_SUBJECT, 0);
		return _subjectList[0].ref;
	}

	byte *icon;

	if (!_choosing) {
		// This is a new conversation menu.

		if (!in_subject)
			error("fnChoose with no subjects");

		for (i = 0; i < in_subject; i++) {
			icon = _vm->_resman->openResource(_subjectList[i].res) + ResHeader::size() + menuIconWidth * RDMENU_ICONDEEP;
			setMenuIcon(RDMENU_BOTTOM, i, icon);
			_vm->_resman->closeResource(_subjectList[i].res);
		}

		for (; i < 15; i++)
			setMenuIcon(RDMENU_BOTTOM, (uint8) i, NULL);

		showMenu(RDMENU_BOTTOM);
		setMouse(NORMAL_MOUSE_ID);
		_choosing = true;
		return (uint32)-1;
	}

	// The menu is there - we're just waiting for a click. We only care
	// about left clicks.

	MouseEvent *me = _vm->mouseEvent();
	int mouseX, mouseY;

	getPos(mouseX, mouseY);

	if (!me || !(me->buttons & RD_LEFTBUTTONDOWN) || mouseY < 400)
		return (uint32)-1;

	// Check for click on a menu.

	int hit = _vm->_mouse->menuClick(in_subject);
	if (hit < 0)
		return (uint32)-1;

	// Hilight the clicked icon by greying the others. This can look a bit
	// odd when you click on the exit icon, but there are also cases when
	// it looks strange if you don't do it.

	for (i = 0; i < in_subject; i++) {
		if ((int)i != hit) {
			icon = _vm->_resman->openResource(_subjectList[i].res) + ResHeader::size();
			_vm->_mouse->setMenuIcon(RDMENU_BOTTOM, i, icon);
			_vm->_resman->closeResource(_subjectList[i].res);
		}
	}

	// For non-speech scripts that manually call the chooser
	_vm->_logic->writeVar(RESULT, _subjectList[hit].res);

	// The conversation menu will be closed

	_choosing = false;
	_vm->_logic->writeVar(IN_SUBJECT, 0);
	setMouse(0);

	return _subjectList[hit].ref;
}

void Mouse::mouseOnOff() {
	// this handles the cursor graphic when moving on and off mouse areas
	// it also handles the luggage thingy

	uint32 pointer_type;
	static uint8 mouse_flicked_off = 0;

	_oldMouseTouching = _mouseTouching;

	// don't detect objects that are hidden behind the menu bars (ie. in
	// the scrolled-off areas of the screen)

	int y = getY();

	if (y < 0 || y > 399) {
		pointer_type = 0;
		_mouseTouching = 0;
	} else {
		// set '_mouseTouching' & return pointer_type
		pointer_type = checkMouseList();
	}

	// same as previous cycle?
	if (!mouse_flicked_off && _oldMouseTouching == _mouseTouching) {
		// yes, so nothing to do
		// BUT CARRY ON IF MOUSE WAS FLICKED OFF!
		return;
	}

	// can reset this now
	mouse_flicked_off = 0;

	//the cursor has moved onto something
	if (!_oldMouseTouching && _mouseTouching) {
		// make a copy of the object we've moved onto because one day
		// we'll move back off again! (but the list positioning could
		// theoretically have changed)

		// we can only move onto something from being on nothing - we
		// stop the system going from one to another when objects
		// overlap

		_oldMouseTouching = _mouseTouching;

		// run get on

		if (pointer_type) {
			// 'pointer_type' holds the resource id of the
			// pointer anim

			setMouse(pointer_type);

			// setup luggage icon
			if (_vm->_logic->readVar(OBJECT_HELD)) {
				setLuggage(_currentLuggageResource);
			}
		} else {
			error("ERROR: mouse.pointer==0 for object %d (%s) - update logic script", _mouseTouching, _vm->_resman->fetchName(_mouseTouching));
		}
	} else if (_oldMouseTouching && !_mouseTouching) {
		// the cursor has moved off something - reset cursor to
		// normal pointer

		_oldMouseTouching = 0;
		setMouse(NORMAL_MOUSE_ID);

		// reset luggage only when necessary
	} else if (_oldMouseTouching && _mouseTouching) {
		// The cursor has moved off something and onto something
		// else. Flip to a blank cursor for a cycle.

		// ignore the new id this cycle - should hit next cycle
		_mouseTouching = 0;
		_oldMouseTouching = 0;
		setMouse(0);

		// so we know to set the mouse pointer back to normal if 2nd
		// hot-spot doesn't register because mouse pulled away
		// quickly (onto nothing)

		mouse_flicked_off = 1;

		// reset luggage only when necessary
	} else {
		// Mouse was flicked off for one cycle, but then moved onto
		// nothing before 2nd hot-spot registered

		// both '_oldMouseTouching' & '_mouseTouching' will be zero
		// reset cursor to normal pointer

		setMouse(NORMAL_MOUSE_ID);
	}

	// possible check for edge of screen more-to-scroll here on large
	// screens
}

void Mouse::setMouse(uint32 res) {
	// high level - whats the mouse - for the engine
	_mousePointerRes = res;

	if (res) {
		byte *icon = _vm->_resman->openResource(res) + ResHeader::size();
		uint32 len = _vm->_resman->fetchLen(res) - ResHeader::size();

		// don't pulse the normal pointer - just do the regular anim
		// loop

		if (res == NORMAL_MOUSE_ID)
			setMouseAnim(icon, len, RDMOUSE_NOFLASH);
		else
			setMouseAnim(icon, len, RDMOUSE_FLASH);

		_vm->_resman->closeResource(res);
	} else {
		// blank cursor
		setMouseAnim(NULL, 0, 0);
	}
}

void Mouse::setLuggage(uint32 res) {
	_realLuggageItem = res;

	if (res) {
		byte *icon = _vm->_resman->openResource(res) + ResHeader::size();
		uint32 len = _vm->_resman->fetchLen(res) - ResHeader::size();

		setLuggageAnim(icon, len);
		_vm->_resman->closeResource(res);
	} else
		setLuggageAnim(NULL, 0);
}

void Mouse::setObjectHeld(uint32 res) {
	setLuggage(res);

	_vm->_logic->writeVar(OBJECT_HELD, res);
	_currentLuggageResource = res;

	// mode locked - no menu available
	_mouseModeLocked = true;
}

uint32 Mouse::checkMouseList() {
	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();
	int x, y;

	getPos(x, y);

	Common::Point mousePos(x + screenInfo->scroll_offset_x, y + screenInfo->scroll_offset_y);

	// Number of priorities subject to implementation needs
	for (int priority = 0; priority < 10; priority++) {
		for (uint i = 0; i < _curMouse; i++) {
			// If the mouse pointer is over this
			// mouse-detection-box

			if (_mouseList[i].priority == priority && _mouseList[i].rect.contains(mousePos)) {
				// Record id
				_mouseTouching = _mouseList[i].id;

				createPointerText(_mouseList[i].pointer_text, _mouseList[i].pointer);

				// Return pointer type
				return _mouseList[i].pointer;
			}
		}
	}

	// Touching nothing; no pointer to return
	_mouseTouching = 0;
	return 0;
}

#define POINTER_TEXT_WIDTH	640		// just in case!
#define POINTER_TEXT_PEN	184		// white

void Mouse::createPointerText(uint32 text_id, uint32 pointer_res) {
	uint32 local_text;
	uint32 text_res;
	byte *text;
	// offsets for pointer text sprite from pointer position
	int16 xOffset, yOffset;
	uint8 justification;

	if (!_objectLabels || !text_id)
		return;

	// Check what the pointer is, to set offsets correctly for text
	// position

	switch (pointer_res) {
	case CROSHAIR:
		yOffset = -7;
		xOffset = +10;
		break;
	case EXIT0:
		yOffset = +15;
		xOffset = +20;
		break;
	case EXIT1:
		yOffset = +16;
		xOffset = -10;
		break;
	case EXIT2:
		yOffset = +10;
		xOffset = -22;
		break;
	case EXIT3:
		yOffset = -16;
		xOffset = -10;
		break;
	case EXIT4:
		yOffset = -15;
		xOffset = +15;
		break;
	case EXIT5:
		yOffset = -12;
		xOffset = +10;
		break;
	case EXIT6:
		yOffset = +10;
		xOffset = +25;
		break;
	case EXIT7:
		yOffset = +16;
		xOffset = +20;
		break;
	case EXITDOWN:
		yOffset = -20;
		xOffset = -10;
		break;
	case EXITUP:
		yOffset = +20;
		xOffset = +20;
		break;
	case MOUTH:
		yOffset = -10;
		xOffset = +15;
		break;
	case NORMAL:
		yOffset = -10;
		xOffset = +15;
		break;
	case PICKUP:
		yOffset = -40;
		xOffset = +10;
		break;
	case SCROLL_L:
		yOffset = -20;
		xOffset = +20;
		break;
	case SCROLL_R:
		yOffset = -20;
		xOffset = -20;
		break;
	case USE:
		yOffset = -8;
		xOffset = +20;
		break;
	default:
		// Shouldn't happen if we cover all the different mouse
		// pointers above
		yOffset = -10;
		xOffset = +10;
		break;
	}

	// Set up justification for text sprite, based on its offsets from the
	// pointer position

	if (yOffset < 0) {
		// Above pointer
		if (xOffset < 0) {
			// Above left
			justification = POSITION_AT_RIGHT_OF_BASE;
		} else if (xOffset > 0) {
			// Above right
			justification = POSITION_AT_LEFT_OF_BASE;
		} else {
			// Above center
			justification = POSITION_AT_CENTER_OF_BASE;
		}
	} else if (yOffset > 0) {
		// Below pointer
		if (xOffset < 0) {
			// Below left
			justification = POSITION_AT_RIGHT_OF_TOP;
		} else if (xOffset > 0) {
			// Below right
			justification = POSITION_AT_LEFT_OF_TOP;
		} else {
			// Below center
			justification = POSITION_AT_CENTER_OF_TOP;
		}
	} else {
		// Same y-coord as pointer
		if (xOffset < 0) {
			// Center left
			justification = POSITION_AT_RIGHT_OF_CENTER;
		} else if (xOffset > 0) {
			// Center right
			justification = POSITION_AT_LEFT_OF_CENTER;
		} else {
			// Center center - shouldn't happen anyway!
			justification = POSITION_AT_LEFT_OF_CENTER;
		}
	}

	// Text resource number, and line number within the resource

	text_res = text_id / SIZE;
	local_text = text_id & 0xffff;

	// open text file & get the line
	text = _vm->fetchTextLine(_vm->_resman->openResource(text_res), local_text);

	// 'text+2' to skip the first 2 bytes which form the
	// line reference number

	int x, y;

	getPos(x, y);

	_pointerTextBlocNo = _vm->_fontRenderer->buildNewBloc(
		text + 2, x + xOffset,
		y + yOffset,
		POINTER_TEXT_WIDTH, POINTER_TEXT_PEN,
		RDSPR_TRANS | RDSPR_DISPLAYALIGN,
		_vm->_speechFontId, justification);

	// now ok to close the text file
	_vm->_resman->closeResource(text_res);
}

void Mouse::clearPointerText() {
	if (_pointerTextBlocNo) {
		_vm->_fontRenderer->killTextBloc(_pointerTextBlocNo);
		_pointerTextBlocNo = 0;
	}
}

void Mouse::hideMouse() {
	// leaves the menus open
	// used by the system when clicking right on a menu item to examine
	// it and when combining objects

	// for logic scripts
	_vm->_logic->writeVar(MOUSE_AVAILABLE, 0);

	// human/mouse off
	_mouseStatus = true;

	setMouse(0);
	setLuggage(0);
}

void Mouse::noHuman() {
	hideMouse();
	clearPointerText();

	// Must be normal mouse situation or a largely neutral situation -
	// special menus use hideMouse()

	// Don't hide menu in conversations
	if (_vm->_logic->readVar(TALK_FLAG) == 0)
		hideMenu(RDMENU_BOTTOM);

	if (_mouseMode == MOUSE_system_menu) {
		// Close menu
		_mouseMode = MOUSE_normal;
		hideMenu(RDMENU_TOP);
	}
}

void Mouse::addHuman() {
	// For logic scripts
	_vm->_logic->writeVar(MOUSE_AVAILABLE, 1);

	if (_mouseStatus) {
		// Force engine to choose a cursor
		_mouseStatus = false;
		_mouseTouching = 1;
	}

	// Clear this to reset no-second-click system
	_vm->_logic->writeVar(CLICKED_ID, 0);

	// This is now done outside the OBJECT_HELD check in case it's set to
	// zero before now!

	// Unlock the mouse from possible large object lock situtations - see
	// syphon in rm 3

	_mouseModeLocked = false;

	if (_vm->_logic->readVar(OBJECT_HELD)) {
		// Was dragging something around - need to clear this again
		_vm->_logic->writeVar(OBJECT_HELD, 0);

		// And these may also need clearing, just in case
		_examiningMenuIcon = false;
		_vm->_logic->writeVar(COMBINE_BASE, 0);

		setLuggage(0);
	}

	// If mouse is over menu area
	if (getY() > 399) {
		if (_mouseMode != MOUSE_holding) {
			// VITAL - reset things & rebuild the menu
			_mouseMode = MOUSE_normal;
		}
		setMouse(NORMAL_MOUSE_ID);
	}

	// Enabled/disabled from console; status printed with on-screen debug
	// info

	if (_vm->_debugger->_testingSnR) {
		uint8 black[3] = {   0,  0,    0 };
		uint8 white[3] = { 255, 255, 255 };

		// Testing logic scripts by simulating instant Save & Restore

		_vm->_screen->setPalette(0, 1, white, RDPAL_INSTANT);

		// Stops all fx & clears the queue - eg. when leaving a room
		_vm->_sound->clearFxQueue(false);

		// Trash all object resources so they load in fresh & restart
		// their logic scripts

		_vm->_resman->killAllObjects(false);

		_vm->_screen->setPalette(0, 1, black, RDPAL_INSTANT);
	}
}

void Mouse::refreshInventory() {
	// Can reset this now
	_vm->_logic->writeVar(COMBINE_BASE, 0);

	// Cause 'object_held' icon to be greyed. The rest are colored.
	_examiningMenuIcon = true;
	buildMenu();
	_examiningMenuIcon = false;
}

void Mouse::startConversation() {
	if (_vm->_logic->readVar(TALK_FLAG) == 0) {
		// See fnChooser & speech scripts
		_vm->_logic->writeVar(CHOOSER_COUNT_FLAG, 0);
	}

	noHuman();
}

void Mouse::endConversation() {
	hideMenu(RDMENU_BOTTOM);

	if (getY() > 399) {
		// Will wait for cursor to move off the bottom menu
		_mouseMode = MOUSE_holding;
	}

	// In case DC forgets
	_vm->_logic->writeVar(TALK_FLAG, 0);
}

void Mouse::monitorPlayerActivity() {
	// if there is at least one mouse event outstanding
	if (_vm->checkForMouseEvents()) {
		// reset activity delay counter
		_playerActivityDelay = 0;
	} else {
		// no. of game cycles since mouse event queue last empty
		_playerActivityDelay++;
	}
}

void Mouse::checkPlayerActivity(uint32 seconds) {
	// Convert seconds to game cycles
	uint32 threshold = seconds * 12;

	// If the actual delay is at or above the given threshold, reset the
	// activity delay counter now that we've got a positive check.

	if (_playerActivityDelay >= threshold) {
		_playerActivityDelay = 0;
		_vm->_logic->writeVar(RESULT, 1);
	} else
		_vm->_logic->writeVar(RESULT, 0);
}

void Mouse::pauseEngine(bool pause) {
	if (pause) {
		// Make the mouse cursor normal. This is the only place where
		// we are allowed to clear the luggage this way.

		clearPointerText();
		setLuggageAnim(NULL, 0);
		setMouse(0);
		setMouseTouching(1);
	} else {
		if (_vm->_logic->readVar(OBJECT_HELD) && _realLuggageItem)
			setLuggage(_realLuggageItem);
	}
}

#define MOUSEFLASHFRAME 6

void Mouse::decompressMouse(byte *decomp, byte *comp, uint8 frame, int width, int height, int pitch, int xOff, int yOff) {
	int32 size = width * height;
	int32 i = 0;
	int x = 0;
	int y = 0;

	if (Sword2Engine::isPsx()) {
		comp = comp + READ_LE_UINT32(comp + 2 + frame * 4) - MOUSE_ANIM_HEADER_SIZE;

		yOff /= 2; // Without this, distance of object from cursor is too big.

		byte *buffer;

		buffer = (byte *)malloc(size);
		Screen::decompressHIF(comp, buffer);

		for (int line = 0; line < height; line++) {
			memcpy(decomp + (line + yOff) * pitch + xOff, buffer + line * width, width);
		}

		free(buffer);

	} else {
		comp = comp + READ_LE_UINT32(comp + frame * 4) - MOUSE_ANIM_HEADER_SIZE;

		while (i < size) {
			if (*comp > 183) {
				decomp[(y + yOff) * pitch + x + xOff] = *comp++;
				if (++x >= width) {
					x = 0;
					y++;
				}
				i++;
			} else {
				x += *comp;
				while (x >= width) {
					y++;
					x -= width;
				}
				i += *comp++;
			}
		}
	}
}

void Mouse::drawMouse() {
	if (!_mouseAnim.data && !_luggageAnim.data)
		return;

	// When an object is used in the game, the mouse cursor should be a
	// combination of a standard mouse cursor and a luggage cursor.
	//
	// However, judging by the original code luggage cursors can also
	// appear on their own. I have no idea which cases though.

	uint16 mouse_width = 0;
	uint16 mouse_height = 0;
	uint16 hotspot_x = 0;
	uint16 hotspot_y = 0;
	int deltaX = 0;
	int deltaY = 0;

	if (_mouseAnim.data) {
		hotspot_x = _mouseAnim.xHotSpot;
		hotspot_y = _mouseAnim.yHotSpot;
		mouse_width = _mouseAnim.mousew;
		mouse_height = _mouseAnim.mouseh;
	}

	if (_luggageAnim.data) {
		if (!_mouseAnim.data) {
			hotspot_x = _luggageAnim.xHotSpot;
			hotspot_y = _luggageAnim.yHotSpot;
		}
		if (_luggageAnim.mousew > mouse_width)
			mouse_width = _luggageAnim.mousew;
		if (_luggageAnim.mouseh > mouse_height)
			mouse_height = _luggageAnim.mouseh;
	}

	if (_mouseAnim.data && _luggageAnim.data) {
		deltaX = _mouseAnim.xHotSpot - _luggageAnim.xHotSpot;
		deltaY = _mouseAnim.yHotSpot - _luggageAnim.yHotSpot;
	}

	assert(deltaX >= 0);
	assert(deltaY >= 0);

	mouse_width += deltaX;
	mouse_height += deltaY;

	byte *mouseData = (byte *)calloc(mouse_height, mouse_width);

	if (_luggageAnim.data)
		decompressMouse(mouseData, _luggageAnim.data, 0,
			_luggageAnim.mousew, _luggageAnim.mouseh,
			mouse_width, deltaX, deltaY);

	if (_mouseAnim.data)
		decompressMouse(mouseData, _mouseAnim.data, _mouseFrame,
			_mouseAnim.mousew, _mouseAnim.mouseh, mouse_width);

	// Fix height for mouse sprite in PSX version
	if (Sword2Engine::isPsx()) {
		mouse_height *= 2;

		byte *buffer = (byte *)malloc(mouse_width * mouse_height);
		Screen::resizePsxSprite(buffer, mouseData, mouse_width, mouse_height);

		free(mouseData);
		mouseData = buffer;
	}

	CursorMan.replaceCursor(mouseData, mouse_width, mouse_height, hotspot_x, hotspot_y, 0);

	free(mouseData);
}

/**
 * Animates the current mouse pointer
 */

int32 Mouse::animateMouse() {
	uint8 prevMouseFrame = _mouseFrame;

	if (!_mouseAnim.data)
		return RDERR_UNKNOWN;

	if (++_mouseFrame == _mouseAnim.noAnimFrames)
		_mouseFrame = MOUSEFLASHFRAME;

	if (_mouseFrame != prevMouseFrame)
		drawMouse();

	return RD_OK;
}

/**
 * Sets the mouse cursor animation.
 * @param ma a pointer to the animation data, or NULL to clear the current one
 * @param size the size of the mouse animation data
 * @param mouseFlash RDMOUSE_FLASH or RDMOUSE_NOFLASH, depending on whether
 * or not there is a lead-in animation
 */

int32 Mouse::setMouseAnim(byte *ma, int32 size, int32 mouseFlash) {
	free(_mouseAnim.data);
	_mouseAnim.data = NULL;

	if (ma)	{
		if (mouseFlash == RDMOUSE_FLASH)
			_mouseFrame = 0;
		else
			_mouseFrame = MOUSEFLASHFRAME;

		Common::MemoryReadStream readS(ma, size);

		_mouseAnim.runTimeComp = readS.readByte();
		_mouseAnim.noAnimFrames = readS.readByte();
		_mouseAnim.xHotSpot = readS.readSByte();
		_mouseAnim.yHotSpot = readS.readSByte();
		_mouseAnim.mousew = readS.readByte();
		_mouseAnim.mouseh = readS.readByte();

		_mouseAnim.data = (byte *)malloc(size - MOUSE_ANIM_HEADER_SIZE);
		if (!_mouseAnim.data)
			return RDERR_OUTOFMEMORY;

		readS.read(_mouseAnim.data, size - MOUSE_ANIM_HEADER_SIZE);

		animateMouse();
		drawMouse();

		CursorMan.showMouse(true);
	} else {
		if (_luggageAnim.data)
			drawMouse();
		else
			CursorMan.showMouse(false);
	}

	return RD_OK;
}

/**
 * Sets the "luggage" animation to accompany the mouse animation. Luggage
 * sprites are of the same format as mouse sprites.
 * @param ma a pointer to the animation data, or NULL to clear the current one
 * @param size the size of the animation data
 */

int32 Mouse::setLuggageAnim(byte *ma, int32 size) {
	free(_luggageAnim.data);
	_luggageAnim.data = NULL;

	if (ma)	{
		Common::MemoryReadStream readS(ma, size);

		_luggageAnim.runTimeComp = readS.readByte();
		_luggageAnim.noAnimFrames = readS.readByte();
		_luggageAnim.xHotSpot = readS.readSByte();
		_luggageAnim.yHotSpot = readS.readSByte();
		_luggageAnim.mousew = readS.readByte();
		_luggageAnim.mouseh = readS.readByte();

		_luggageAnim.data = (byte *)malloc(size - MOUSE_ANIM_HEADER_SIZE);
		if (!_luggageAnim.data)
			return RDERR_OUTOFMEMORY;

		readS.read(_luggageAnim.data, size - MOUSE_ANIM_HEADER_SIZE);

		animateMouse();
		drawMouse();

		CursorMan.showMouse(true);
	} else {
		if (_mouseAnim.data)
			drawMouse();
		else
			CursorMan.showMouse(false);
	}

	return RD_OK;
}

int Mouse::getMouseMode() {
	return _mouseMode;
}

void Mouse::setMouseMode(int mouseMode) {
	_mouseMode = mouseMode;
}

} // End of namespace Sword2
