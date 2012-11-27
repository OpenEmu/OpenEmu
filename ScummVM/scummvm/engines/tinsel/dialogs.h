
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
 * Inventory related functions
 */

#ifndef TINSEL_INVENTORY_H	// prevent multiple includes
#define TINSEL_INVENTORY_H

#include "tinsel/dw.h"
#include "tinsel/events.h"	// for PLR_EVENT, PLR_EVENT

namespace Common {
class Serializer;
}

namespace Tinsel {

enum {
	INV_OPEN	= -1,	// DW1 only
	INV_CONV	= 0,
	INV_1		= 1,
	INV_2		= 2,
	INV_CONF	= 3,
	INV_MENU	= 3,	// DW2 constant
	NUM_INV		= 4,

	// Discworld 2 constants
	DW2_INV_OPEN = 5,
	INV_DEFAULT = 6
};

/** structure of each inventory object */
struct INV_OBJECT {
	int32 id;		// inventory objects id
	SCNHANDLE hIconFilm;	// inventory objects animation film
	SCNHANDLE hScript;	// inventory objects event handling script
	int32 attribute;		// inventory object's attribute
};

// attribute values - not a bit bit field to prevent portability problems
#define DROPCODE	0x01
#define ONLYINV1	0x02
#define ONLYINV2	0x04
#define DEFINV1		0x08
#define DEFINV2		0x10
#define PERMACONV	0x20
#define CONVENDITEM	0x40

void PopUpInventory(int invno);

enum CONFTYPE {
	MAIN_MENU, SAVE_MENU, LOAD_MENU, QUIT_MENU, RESTART_MENU, SOUND_MENU,
	CONTROLS_MENU, SUBTITLES_MENU, HOPPER_MENU1, HOPPER_MENU2, TOP_WINDOW
};

void OpenMenu(CONFTYPE type);


void Xmovement(int x);
void Ymovement(int y);

void EventToInventory(PLR_EVENT pEvent, const Common::Point &coOrds);
void ButtonToInventory(PLR_EVENT be);
void KeyToInventory(PLR_EVENT ke);


int WhichItemHeld();

void HoldItem(int item, bool bKeepFilm = false);
void DropItem(int item);
void ClearInventory(int invno);
void AddToInventory(int invno, int icon, bool hold = false);
bool RemFromInventory(int invno, int icon);


void RegisterIcons(void *cptr, int num);

void idec_convw(SCNHANDLE text, int MaxContents, int MinWidth, int MinHeight,
			int StartWidth, int StartHeight, int MaxWidth, int MaxHeight);
void idec_inv1(SCNHANDLE text, int MaxContents, int MinWidth, int MinHeight,
			int StartWidth, int StartHeight, int MaxWidth, int MaxHeight);
void idec_inv2(SCNHANDLE text, int MaxContents, int MinWidth, int MinHeight,
			int StartWidth, int StartHeight, int MaxWidth, int MaxHeight);

bool InventoryActive();

void PermaConvIcon(int icon, bool bEnd = false);

void convPos(int bpos);
void ConvPoly(HPOLYGON hp);
int GetIcon();
void CloseDownConv();
void HideConversation(bool hide);
bool ConvIsHidden();

enum {
	NOOBJECT		= -1,
	INV_NOICON		= -1,
	INV_CLOSEICON	= -2,
	INV_OPENICON	= -3,
	INV_HELDNOTIN	= -4
};

enum CONV_PARAM {
	CONV_DEF,
	CONV_BOTTOM,
	CONV_END,
	CONV_TOP
};


void ConvAction(int index);
void SetConvDetails(CONV_PARAM fn, HPOLYGON hPoly, int ano);
void InventoryIconCursor(bool bNewItem);

void setInvWinParts(SCNHANDLE hf);
void setFlagFilms(SCNHANDLE hf);
void setConfigStrings(SCNHANDLE *tp);

int InvItem(int *x, int *y, bool update);
int InvItemId(int x, int y);

int InventoryPos(int num);

bool IsInInventory(int object, int invnum);

void KillInventory();

void syncInvInfo(Common::Serializer &s);

int InvGetLimit(int invno);
void InvSetLimit(int invno, int n);
void InvSetSize(int invno, int MinWidth, int MinHeight,
		int StartWidth, int StartHeight, int MaxWidth, int MaxHeight);

bool GetIsInvObject(int id);

int WhichInventoryOpen();

bool IsTopWindow();
bool MenuActive();
bool IsConvWindow();

void SetObjectFilm(int object, SCNHANDLE hFilm);

void ObjectEvent(CORO_PARAM, int objId, TINSEL_EVENT event, bool bWait, int myEscape, bool *result = NULL);

} // End of namespace Tinsel

#endif /* TINSEL_INVENTRY_H */
