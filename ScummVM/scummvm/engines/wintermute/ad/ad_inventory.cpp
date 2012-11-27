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
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/ad/ad_inventory.h"
#include "engines/wintermute/ad/ad_game.h"
#include "engines/wintermute/ad/ad_item.h"
#include "engines/wintermute/platform_osystem.h"
#include "common/str.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdInventory, false)

//////////////////////////////////////////////////////////////////////////
AdInventory::AdInventory(BaseGame *inGame) : BaseObject(inGame) {
	_scrollOffset = 0;
}


//////////////////////////////////////////////////////////////////////////
AdInventory::~AdInventory() {
	_takenItems.clear(); // ref only
}


//////////////////////////////////////////////////////////////////////////
bool AdInventory::insertItem(const char *name, const char *insertAfter) {
	if (name == NULL) {
		return STATUS_FAILED;
	}

	AdItem *item = ((AdGame *)_gameRef)->getItemByName(name);
	if (item == NULL) {
		return STATUS_FAILED;
	}

	int insertIndex = -1;
	for (uint32 i = 0; i < _takenItems.size(); i++) {
		if (scumm_stricmp(_takenItems[i]->getName(), name) == 0) {
			_takenItems.remove_at(i);
			i--;
			continue;
		}
		if (insertAfter && scumm_stricmp(_takenItems[i]->getName(), insertAfter) == 0) {
			insertIndex = i + 1;
		}
	}


	if (insertIndex == -1) {
		_takenItems.add(item);
	} else {
		_takenItems.insert_at(insertIndex, item);
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdInventory::removeItem(const char *name) {
	if (name == NULL) {
		return STATUS_FAILED;
	}

	for (uint32 i = 0; i < _takenItems.size(); i++) {
		if (scumm_stricmp(_takenItems[i]->getName(), name) == 0) {
			if (((AdGame *)_gameRef)->_selectedItem == _takenItems[i]) {
				((AdGame *)_gameRef)->_selectedItem = NULL;
			}
			_takenItems.remove_at(i);
			return STATUS_OK;
		}
	}

	return STATUS_FAILED;
}



//////////////////////////////////////////////////////////////////////////
bool AdInventory::removeItem(AdItem *item) {
	if (item == NULL) {
		return STATUS_FAILED;
	}

	for (uint32 i = 0; i < _takenItems.size(); i++) {
		if (_takenItems[i] == item) {
			if (((AdGame *)_gameRef)->_selectedItem == _takenItems[i]) {
				((AdGame *)_gameRef)->_selectedItem = NULL;
			}
			_takenItems.remove_at(i);
			return STATUS_OK;
		}
	}

	return STATUS_FAILED;
}

//////////////////////////////////////////////////////////////////////////
bool AdInventory::persist(BasePersistenceManager *persistMgr) {

	BaseObject::persist(persistMgr);

	_takenItems.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_scrollOffset));

	return STATUS_OK;
}

} // end of namespace Wintermute
