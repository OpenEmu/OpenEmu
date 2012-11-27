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

#ifndef SWORD2_MOUSE_H
#define SWORD2_MOUSE_H

#include "common/rect.h"

#define MENU_MASTER_OBJECT	44

#define	MAX_SUBJECT_LIST 30	// is that enough?

#define TOTAL_subjects		(375 - 256 + 1)	// the speech subject bar
#define TOTAL_engine_pockets	(15 + 10)	// +10 for overflow
#define	TOTAL_mouse_list 50

namespace Sword2 {

struct MenuObject;
struct BuildUnit;

// Menubar defines.

#define RDMENU_TOP		0
#define RDMENU_BOTTOM		1

enum {
	MOUSE_normal		= 0,	// normal in game
	MOUSE_menu		= 1,	// menu chooser
	MOUSE_drag		= 2,	// dragging luggage
	MOUSE_system_menu	= 3,	// system menu chooser
	MOUSE_holding		= 4	// special
};

enum {
	RDMOUSE_NOFLASH,
	RDMOUSE_FLASH
};

enum {
	RDMENU_HIDDEN,
	RDMENU_SHOWN,
	RDMENU_OPENING,
	RDMENU_CLOSING,
	RDMENU_ALMOST_HIDDEN
};

#define RDMENU_ICONWIDE		35
#define RDMENU_PSXICONWIDE	36
#define RDMENU_ICONDEEP		30
#define RDMENU_ICONSTART	24
#define RDMENU_ICONSPACING	5
#define RDMENU_MAXPOCKETS	15

#define MOUSE_ANIM_HEADER_SIZE	6

struct MouseAnim {
	uint8 runTimeComp;	// type of runtime compression used for the
				// frame data
	uint8 noAnimFrames;	// number of frames in the anim
	int8 xHotSpot;
	int8 yHotSpot;
	uint8 mousew;
	uint8 mouseh;

	byte *data;
};

// The MOUSE_holding mode is entered when the conversation menu is closed, and
// exited when the mouse cursor moves off that menu area. I don't know why yet.

// mouse unit - like ObjectMouse, but with anim resource & pc (needed if
// sprite is to act as mouse detection mask)

struct MouseUnit {
	// Basically the same information as in ObjectMouse, except the
	// coordinates are adjusted to conform to standard ScummVM usage.

	Common::Rect rect;
	int32 priority;
	int32 pointer;

	// In addition, we need an id when checking the mouse list, and a
	// text id for mouse-overs.

	int32 id;
	int32 pointer_text;
};

// Array of these for subject menu build up

 struct SubjectUnit {
	uint32 res;
	uint32 ref;
};

// define these in a script and then register them with the system

struct MenuObject {
	int32 icon_resource;	// icon graphic graphic
	int32 luggage_resource;	// luggage icon resource (for attaching to
				// mouse pointer)
};

class Mouse {
private:
	Sword2Engine *_vm;

	MouseUnit _mouseList[TOTAL_mouse_list];
	uint32 _curMouse;

	MenuObject _tempList[TOTAL_engine_pockets];
	uint32 _totalTemp;

	MenuObject _masterMenuList[TOTAL_engine_pockets];
	uint32 _totalMasters;

	SubjectUnit _subjectList[MAX_SUBJECT_LIST];

	// ref number for default response when luggage icon is used on a
	// person & it doesn't match any of the icons which would have been in
	// the chooser

	uint32 _defaultResponseId;

	// could alternately use logic->looping of course
	bool _choosing;

	uint8 _menuStatus[2];
	byte *_icons[2][RDMENU_MAXPOCKETS];
	uint8 _pocketStatus[2][RDMENU_MAXPOCKETS];

	uint8 _iconCount;

	// If it's NORMAL_MOUSE_ID (ie. normal pointer) then it's over a floor
	// area (or hidden hot-zone)

	uint32 _mousePointerRes;

	MouseAnim _mouseAnim;
	MouseAnim _luggageAnim;

	uint8 _mouseFrame;

	uint32 _mouseMode;

	bool _mouseStatus;		// Human 0 on/1 off
	bool _mouseModeLocked;		// 0 not !0 mode cannot be changed from
					// normal mouse to top menu (i.e. when
					// carrying big objects)
	uint32 _realLuggageItem;	// Last minute for pause mode
	uint32 _currentLuggageResource;
	uint32 _oldButton;		// For the re-click stuff - must be
					// the same button you see
	uint32 _buttonClick;
	uint32 _pointerTextBlocNo;
	uint32 _playerActivityDelay;	// Player activity delay counter

	bool _examiningMenuIcon;

	// Set by checkMouseList()
	uint32 _mouseTouching;
	uint32 _oldMouseTouching;

	bool _objectLabels;

	uint32 _menuSelectedPos;

	void decompressMouse(byte *decomp, byte *comp, uint8 frame, int width, int height, int pitch, int xOff = 0, int yOff = 0);

	int32 setMouseAnim(byte *ma, int32 size, int32 mouseFlash);
	int32 setLuggageAnim(byte *la, int32 size);

	void clearIconArea(int menu, int pocket, Common::Rect *r);

public:
	Mouse(Sword2Engine *vm);
	~Mouse();

	void getPos(int &x, int &y);
	int getX();
	int getY();

	bool getObjectLabels() { return _objectLabels; }
	void setObjectLabels(bool b) { _objectLabels = b; }

	bool getMouseStatus() { return _mouseStatus; }
	uint32 getMouseTouching() { return _mouseTouching; }
	void setMouseTouching(uint32 touching) { _mouseTouching = touching; }

	void pauseEngine(bool pause);

	void setMouse(uint32 res);
	void setLuggage(uint32 res);

	void setObjectHeld(uint32 res);

	void resetMouseList();

	void registerMouse(byte *ob_mouse, BuildUnit *build_unit);
	void registerPointerText(int32 text_id);

	void createPointerText(uint32 text_id, uint32 pointer_res);
	void clearPointerText();

	void drawMouse();
	int32 animateMouse();

	void processMenu();

	void addMenuObject(byte *ptr);
	void addSubject(int32 id, int32 ref);

	void buildMenu();
	void buildSystemMenu();

	int32 showMenu(uint8 menu);
	int32 hideMenu(uint8 menu);
	int32 setMenuIcon(uint8 menu, uint8 pocket, byte *icon);

	void closeMenuImmediately();

	void refreshInventory();

	void startConversation();
	void endConversation();

	void hideMouse();
	void noHuman();
	void addHuman();

	void resetPlayerActivityDelay() { _playerActivityDelay = 0; }
	void monitorPlayerActivity();
	void checkPlayerActivity(uint32 seconds);

	void mouseOnOff();
	uint32 checkMouseList();
	void mouseEngine();

	void normalMouse();
	void menuMouse();
	void dragMouse();
	void systemMenuMouse();

	bool isChoosing() { return _choosing; }
	uint32 chooseMouse();

	int menuClick(int menu_items);

	int getMouseMode();

	void setMouseMode(int mouseMode); // Used to force mouse mode
};

} // End of namespace Sword2

#endif
