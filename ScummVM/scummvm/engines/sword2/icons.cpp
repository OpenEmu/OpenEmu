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


#include "common/memstream.h"
#include "common/rect.h"

#include "sword2/sword2.h"
#include "sword2/header.h"
#include "sword2/defs.h"
#include "sword2/logic.h"
#include "sword2/mouse.h"
#include "sword2/resman.h"

namespace Sword2 {

void Mouse::addMenuObject(byte *ptr) {
	assert(_totalTemp < TOTAL_engine_pockets);

	Common::MemoryReadStream readS(ptr, 2 * sizeof(int32));

	_tempList[_totalTemp].icon_resource = readS.readSint32LE();
	_tempList[_totalTemp].luggage_resource = readS.readSint32LE();
	_totalTemp++;
}

void Mouse::addSubject(int32 id, int32 ref) {
	uint32 in_subject = _vm->_logic->readVar(IN_SUBJECT);

	if (in_subject == 0) {
		// This is the start of the new subject list. Set the default
		// repsonse id to zero in case we're never passed one.
		_defaultResponseId = 0;
	}

	if (id == -1) {
		// Id -1 is used for setting the default response, i.e. the
		// response when someone uses an object on a person and he
		// doesn't know anything about it. See fnChoose().

		_defaultResponseId = ref;
	} else {
		debug(5, "fnAddSubject res %d, uid %d", id, ref);
		_subjectList[in_subject].res = id;
		_subjectList[in_subject].ref = ref;
		_vm->_logic->writeVar(IN_SUBJECT, in_subject + 1);
	}
}

/**
 * Create and start the inventory (bottom) menu
 */

void Mouse::buildMenu() {
	uint32 i, j;
	byte menuIconWidth;

	if (Sword2Engine::isPsx())
		menuIconWidth = RDMENU_PSXICONWIDE;
	else
		menuIconWidth = RDMENU_ICONWIDE;

	// Clear the temporary inventory list, since we are going to build a
	// new one from scratch.

	for (i = 0; i < TOTAL_engine_pockets; i++)
		_tempList[i].icon_resource = 0;

	_totalTemp = 0;

	// Run the 'build_menu' script in the 'menu_master' object. This will
	// register all carried menu objects.

	_vm->_logic->runResScript(MENU_MASTER_OBJECT, 0);

	// Create a new master list based on the old master inventory list and
	// the new temporary inventory list. The purpose of all this is, as
	// far as I can tell, that the new list is ordered in the same way as
	// the old list, with new objects added to the end of it.

	// Compare new with old. Anything in master thats not in new gets
	// removed from master - if found in new too, remove from temp

	for (i = 0; i < _totalMasters; i++) {
		bool found_in_temp = false;

		for (j = 0; j < TOTAL_engine_pockets; j++) {
			if (_masterMenuList[i].icon_resource == _tempList[j].icon_resource) {
				// We alread know about this object, so kill it
				// in the temporary list.
				_tempList[j].icon_resource = 0;
				found_in_temp = true;
				break;
			}
		}

		if (!found_in_temp) {
			// The object is in the master list, but not in the
			// temporary list. The player must have lost the object
			// since the last time we checked, so kill it in the
			// master list.
			_masterMenuList[i].icon_resource = 0;
		}
	}

	// Eliminate blank entries from the master list.

	_totalMasters = 0;

	for (i = 0; i < TOTAL_engine_pockets; i++) {
		if (_masterMenuList[i].icon_resource) {
			if (i != _totalMasters) {
				memcpy(&_masterMenuList[_totalMasters], &_masterMenuList[i], sizeof(MenuObject));
				_masterMenuList[i].icon_resource = 0;
			}
			_totalMasters++;
		}
	}

	// Add the new objects - i.e. the ones still in the temporary list but
	// not yet in the master list - to the end of the master.

	for (i = 0; i < TOTAL_engine_pockets; i++) {
		if (_tempList[i].icon_resource) {
			memcpy(&_masterMenuList[_totalMasters++], &_tempList[i], sizeof(MenuObject));
		}
	}

	// Initialize the menu from the master list.

	for (i = 0; i < 15; i++) {
		uint32 res = _masterMenuList[i].icon_resource;
		byte *icon = NULL;

		if (res) {
			bool icon_colored;

			uint32 object_held = _vm->_logic->readVar(OBJECT_HELD);
			uint32 combine_base = _vm->_logic->readVar(COMBINE_BASE);

			if (_examiningMenuIcon) {
				// When examining an object, that object is
				// colored. The rest are greyed out.
				icon_colored = (res == object_held);
			} else if (combine_base) {
				// When combining two menu object (i.e. using
				// one on another), both are colored. The rest
				// are greyed out.
				icon_colored = (res == object_held || combine_base);
			} else {
				// If an object is selected but we are not yet
				// doing anything with it, the selected object
				// is greyed out. The rest are colored.
				icon_colored = (res != object_held);
			}

			icon = _vm->_resman->openResource(res) + ResHeader::size();

			// The colored icon is stored directly after the
			// greyed out one.

			if (icon_colored)
				icon += (menuIconWidth * RDMENU_ICONDEEP);
		}

		setMenuIcon(RDMENU_BOTTOM, i, icon);

		if (res)
			_vm->_resman->closeResource(res);
	}

	showMenu(RDMENU_BOTTOM);
}

/**
 * Build a fresh system (top) menu.
 */

void Mouse::buildSystemMenu() {
	uint32 icon_list[5] = {
		OPTIONS_ICON,
		QUIT_ICON,
		SAVE_ICON,
		RESTORE_ICON,
		RESTART_ICON
	};

	byte menuIconWidth;

	if (Sword2Engine::isPsx())
		menuIconWidth = RDMENU_PSXICONWIDE;
	else
		menuIconWidth = RDMENU_ICONWIDE;

	// Build them all high in full color - when one is clicked on all the
	// rest will grey out.

	for (int i = 0; i < ARRAYSIZE(icon_list); i++) {
		byte *icon = _vm->_resman->openResource(icon_list[i]) + ResHeader::size();

		// The only case when an icon is grayed is when the player
		// is dead. Then SAVE is not available.

		if (!_vm->_logic->readVar(DEAD) || icon_list[i] != SAVE_ICON)
			icon += (menuIconWidth * RDMENU_ICONDEEP);

		setMenuIcon(RDMENU_TOP, i, icon);
		_vm->_resman->closeResource(icon_list[i]);
	}

	showMenu(RDMENU_TOP);
}

} // End of namespace Sword2
